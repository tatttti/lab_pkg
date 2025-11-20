const inputSeg = document.getElementById('inputSegments');
const fileSeg = document.getElementById('fileSegments');
const processSegBtn = document.getElementById('processSegments');
const statusSeg = document.getElementById('statusSeg');
const canvasSeg = document.getElementById('canvasSeg');
const ctxSeg = canvasSeg.getContext('2d', { alpha: false });

const inputPoly = document.getElementById('inputPolygon');
const filePoly = document.getElementById('filePolygon');
const processPolyBtn = document.getElementById('processPolygon');
const statusPoly = document.getElementById('statusPoly');
const canvasPoly = document.getElementById('canvasPoly');
const ctxPoly = canvasPoly.getContext('2d', { alpha: false });

let segData = { segments: [], window: { xmin: 2, ymin: 2, xmax: 6, ymax: 6 } };
let polyData = { polygon: [], clipper: [] };

let viewSeg = { x: 0, y: 0, scale: 20 };
let viewPoly = { x: 0, y: 0, scale: 20 };

let panSeg = { active: false, last: null };
let panPoly = { active: false, last: null };

function setStatus(el, text){ el.textContent = text; }

function worldToScreen(view, canvas, pt){
    const cx = canvas.width / 2, cy = canvas.height / 2;
    return { x: cx + (pt.x - view.x) * view.scale, y: cy - (pt.y - view.y) * view.scale };
}

function screenToWorld(view, canvas, px, py){
    const cx = canvas.width / 2, cy = canvas.height / 2;
    return { x: view.x + (px - cx) / view.scale, y: view.y - (py - cy) / view.scale };
}

function chooseGridStep(scale){
    const targetPx = 100;
    const raw = targetPx / scale;
    const steps = [0.5,1,2,5,10,20,50,100,200];
    for(const s of steps) if(s >= raw) return s;
    return steps[steps.length-1];
}

function clearCanvas(ctx, canvas){
    ctx.fillStyle = '#ffffff';
    ctx.fillRect(0,0,canvas.width,canvas.height);
}

function drawGridAndAxes(ctx, canvas, view){
    const step = chooseGridStep(view.scale);
    const topLeft = screenToWorld(view, canvas, 0,0);
    const bottomRight = screenToWorld(view, canvas, canvas.width, canvas.height);
    const xminW = Math.floor(topLeft.x / step) * step;
    const xmaxW = Math.ceil(bottomRight.x / step) * step;
    const yminW = Math.floor(bottomRight.y / step) * step;
    const ymaxW = Math.ceil(topLeft.y / step) * step;

    ctx.save();
    
    ctx.strokeStyle = 'rgba(15,23,42,0.06)';
    ctx.lineWidth = 1;
    ctx.beginPath();
    for(let x = xminW; x <= xmaxW; x += step){
        const a = worldToScreen(view, canvas, {x:x,y:yminW});
        const b = worldToScreen(view, canvas, {x:x,y:ymaxW});
        ctx.moveTo(a.x,a.y); ctx.lineTo(b.x,b.y);
    }
    for(let y = yminW; y <= ymaxW; y += step){
        const a = worldToScreen(view, canvas, {x:xminW,y:y});
        const b = worldToScreen(view, canvas, {x:xmaxW,y:y});
        ctx.moveTo(a.x,a.y); ctx.lineTo(b.x,b.y);
    }
    ctx.stroke();

    ctx.strokeStyle = 'rgba(43,109,246,0.85)';
    ctx.lineWidth = 1.5;
    const xA1 = worldToScreen(view, canvas, {x:xminW, y:0});
    const xA2 = worldToScreen(view, canvas, {x:xmaxW, y:0});
    ctx.beginPath(); ctx.moveTo(xA1.x,xA1.y); ctx.lineTo(xA2.x,xA2.y); ctx.stroke();
    const yA1 = worldToScreen(view, canvas, {x:0,y:yminW});
    const yA2 = worldToScreen(view, canvas, {x:0,y:ymaxW});
    ctx.beginPath(); ctx.moveTo(yA1.x,yA1.y); ctx.lineTo(yA2.x,yA2.y); ctx.stroke();


    ctx.fillStyle = 'rgba(31,41,55,0.8)'; 
    ctx.font = '12px Inter';
    
    for(let x = xminW; x <= xmaxW; x += step){
        if(x === 0) continue;
        const s = worldToScreen(view, canvas, {x:x,y:0});
        ctx.fillText(String(x), s.x + 4, s.y - 4);
    }
    
    for(let y = yminW; y <= ymaxW; y += step){
        if(y === 0) continue;
        const s = worldToScreen(view, canvas, {x:0,y:y});
        ctx.fillText(String(y), s.x + 6, s.y - 2);
    }

    const origin = worldToScreen(view, canvas, {x:0,y:0});
    ctx.fillText('0', origin.x + 4, origin.y - 4);
    
    ctx.restore();
}

function drawWindow(ctx, canvas, view, win){
    const a = worldToScreen(view, canvas, {x:win.xmin,y:win.ymin});
    const b = worldToScreen(view, canvas, {x:win.xmax,y:win.ymax});
    const w = b.x - a.x, h = b.y - a.y;
    ctx.save();
    ctx.fillStyle = 'rgba(14,165,233,0.10)';
    ctx.strokeStyle = '#0ea5e9';
    ctx.lineWidth = 2;
    ctx.fillRect(a.x,a.y,w,h);
    ctx.strokeRect(a.x,a.y,w,h);
    ctx.restore();
}

function drawSegmentWorld(ctx, canvas, view, p1, p2, color, width=2, dash=[]){
    const a = worldToScreen(view, canvas, p1);
    const b = worldToScreen(view, canvas, p2);
    ctx.save();
    ctx.strokeStyle = color;
    ctx.lineWidth = width;
    ctx.setLineDash(dash || []);
    ctx.lineCap = 'round';
    ctx.beginPath(); ctx.moveTo(a.x,a.y); ctx.lineTo(b.x,b.y); ctx.stroke();
    ctx.restore();
}

function drawPolygonWorld(ctx, canvas, view, poly, stroke, fill, width=2){
    if(!poly || poly.length===0) return;
    ctx.save();
    ctx.strokeStyle = stroke;
    ctx.fillStyle = fill;
    ctx.lineWidth = width;
    ctx.beginPath();
    poly.forEach((p,i)=>{
        const s = worldToScreen(view, canvas, p);
        if(i===0) ctx.moveTo(s.x,s.y); else ctx.lineTo(s.x,s.y);
    });
    ctx.closePath();
    ctx.fill(); ctx.stroke();
    ctx.restore();
}

function outCode(p, win){
    let code = 0;
    if(p.x < win.xmin) code |= 1;
    if(p.x > win.xmax) code |= 2;
    if(p.y < win.ymin) code |= 4;
    if(p.y > win.ymax) code |= 8;
    return code;
}

function midpointClipAll(seg, win){
    const result = [];
    const maxDepth = 32;
    function recur(a,b,depth){
        const ca = outCode(a,win), cb = outCode(b,win);
        if((ca | cb) === 0){ result.push({p1:a, p2:b}); return; }
        if((ca & cb) !== 0) return;
        if(depth >= maxDepth){
            const m = { x:(a.x+b.x)/2, y:(a.y+b.y)/2 };
            if(outCode(m,win) === 0) result.push({p1:m, p2:m});
            return;
        }
        const m = { x:(a.x+b.x)/2, y:(a.y+b.y)/2 };
        recur(a,m,depth+1); recur(m,b,depth+1);
    }
    recur(seg.p1, seg.p2, 0);
    return result;
}

function clipPolygon(subjectPolygon, clipPolygon) {
    let outputList = subjectPolygon;
    
    for (let i = 0; i < clipPolygon.length; i++) {
        const inputList = outputList;
        outputList = [];
        
        const A = clipPolygon[i];
        const B = clipPolygon[(i + 1) % clipPolygon.length];
        
        for (let j = 0; j < inputList.length; j++) {
            const P = inputList[j];
            const Q = inputList[(j + 1) % inputList.length];
            
            const Pinside = isInside(P, A, B);
            const Qinside = isInside(Q, A, B);
            
            if (Pinside && Qinside) {
                outputList.push(Q);
            } else if (Pinside && !Qinside) {
                outputList.push(intersection(A, B, P, Q));
            } else if (!Pinside && Qinside) {
                outputList.push(intersection(A, B, P, Q));
                outputList.push(Q);
            }
        }
    }
    
    return outputList;
}

function isInside(point, A, B) {
    return (B.x - A.x) * (point.y - A.y) > (B.y - A.y) * (point.x - A.x);
}

function intersection(A, B, P, Q) {
    const A1 = B.y - A.y;
    const B1 = A.x - B.x;
    const C1 = A1 * A.x + B1 * A.y;
    
    const A2 = Q.y - P.y;
    const B2 = P.x - Q.x;
    const C2 = A2 * P.x + B2 * P.y;
    
    const determinant = A1 * B2 - A2 * B1;
    
    if (determinant === 0) {
        return { x: (P.x + Q.x) / 2, y: (P.y + Q.y) / 2 };
    } else {
        const x = (B2 * C1 - B1 * C2) / determinant;
        const y = (A1 * C2 - A2 * C1) / determinant;
        return { x, y };
    }
}

function renderSegments(){
    ctxSeg.imageSmoothingEnabled = true;
    clearCanvas(ctxSeg, canvasSeg);
    drawGridAndAxes(ctxSeg, canvasSeg, viewSeg);
    drawWindow(ctxSeg, canvasSeg, viewSeg, segData.window);

    const colors = [ '#8b5cf6', '#ef4444', '#fb923c', '#06b6d4', '#10b981' ];
    segData.segments.forEach((s,i)=>{
        drawSegmentWorld(ctxSeg, canvasSeg, viewSeg, s.p1, s.p2, colors[i % colors.length], 2, [6,4]);
    });

    const clippedParts = [];
    segData.segments.forEach(s=>{
        const parts = midpointClipAll(s, segData.window);
        parts.forEach(p=> clippedParts.push(p));
    });

    clippedParts.forEach(s=> drawSegmentWorld(ctxSeg, canvasSeg, viewSeg, s.p1, s.p2, '#16a34a', 3));
    setStatus(statusSeg, `Готово`);
}

function renderPolygon(){
    ctxPoly.imageSmoothingEnabled = true;
    clearCanvas(ctxPoly, canvasPoly);
    drawGridAndAxes(ctxPoly, canvasPoly, viewPoly);
    
    if (polyData.clipper.length > 0) {
        drawPolygonWorld(ctxPoly, canvasPoly, viewPoly, polyData.clipper, '#0ea5e9', 'rgba(14,165,233,0.10)', 2);
    }

    if (polyData.polygon.length > 0) {
        drawPolygonWorld(ctxPoly, canvasPoly, viewPoly, polyData.polygon, '#fb923c', 'rgba(251,146,60,0.16)', 2);
    }

    if (polyData.polygon.length > 0 && polyData.clipper.length > 0) {
        const clipped = clipPolygon(polyData.polygon, polyData.clipper);
        if(clipped.length){
            drawPolygonWorld(ctxPoly, canvasPoly, viewPoly, clipped, '#16a34a', 'rgba(34,197,94,0.22)', 3);
        }
        setStatus(statusPoly, `Готово`);
    } else {
        setStatus(statusPoly, `Готов к работе`);
    }
}

function parseSegmentsText(txt){
    const toks = txt.trim().split(/\s+/).filter(Boolean);
    if(toks.length < 5) throw new Error('Недостаточно данных (часть 1)');
    let idx = 0;
    const n = parseInt(toks[idx++], 10);
    if(Number.isNaN(n) || n < 0) throw new Error('Некорректное число отрезков');
    const segments = [];
    for(let i=0;i<n;i++){
        if(idx+3 >= toks.length) throw new Error('Недостаточно координат для отрезков');
        const x1 = parseFloat(toks[idx++]), y1 = parseFloat(toks[idx++]);
        const x2 = parseFloat(toks[idx++]), y2 = parseFloat(toks[idx++]);
        segments.push({ p1:{x:x1,y:y1}, p2:{x:x2,y:y2} });
    }
    if(idx+3 >= toks.length) throw new Error('Нет координат окна');
    const xmin = parseFloat(toks[idx++]), ymin = parseFloat(toks[idx++]);
    const xmax = parseFloat(toks[idx++]), ymax = parseFloat(toks[idx++]);
    if(xmin >= xmax || ymin >= ymax) throw new Error('Некорректные координаты окна');
    segData.segments = segments;
    segData.window = { xmin, ymin, xmax, ymax };
}

function parsePolygonText(txt){
    const toks = txt.trim().split(/\s+/).filter(Boolean);
    if(toks.length < 3) throw new Error('Недостаточно данных (часть 2)');
    let idx = 0;
    
    const m = parseInt(toks[idx++], 10);
    if(Number.isNaN(m) || m < 3) throw new Error('Число вершин многоугольника должно быть >= 3');
    const poly = [];
    for(let i=0;i<m;i++){
        if(idx+1 >= toks.length) throw new Error('Недостаточно координат вершин многоугольника');
        const x = parseFloat(toks[idx++]), y = parseFloat(toks[idx++]);
        poly.push({ x, y });
    }

    const k = parseInt(toks[idx++], 10);
    if(Number.isNaN(k) || k < 3) throw new Error('Число вершин отсекателя должно быть >= 3');
    const clipper = [];
    for(let i=0;i<k;i++){
        if(idx+1 >= toks.length) throw new Error('Недостаточно координат вершин отсекателя');
        const x = parseFloat(toks[idx++]), y = parseFloat(toks[idx++]);
        clipper.push({ x, y });
    }
    
    polyData.polygon = poly;
    polyData.clipper = clipper;
}

fileSeg.addEventListener('change', e=>{
    const f = e.target.files[0]; if(!f) return;
    const r = new FileReader();
    r.onload = () => { inputSeg.value = r.result; setStatus(statusSeg, 'Файл загружен'); };
    r.readAsText(f);
});

processSegBtn.addEventListener('click', ()=>{
    try{ 
        parseSegmentsText(inputSeg.value); 
        renderSegments(); 
        setStatus(statusSeg, 'Готово'); 
    }
    catch(err){ setStatus(statusSeg, 'Ошибка: '+err.message); }
});

filePoly.addEventListener('change', e=>{
    const f = e.target.files[0]; if(!f) return;
    const r = new FileReader();
    r.onload = () => { inputPoly.value = r.result; setStatus(statusPoly, 'Файл загружен'); };
    r.readAsText(f);
});

processPolyBtn.addEventListener('click', ()=>{
    try{ 
        parsePolygonText(inputPoly.value); 
        renderPolygon(); 
        setStatus(statusPoly, 'Готово'); 
    }
    catch(err){ setStatus(statusPoly, 'Ошибка: '+err.message); }
});

canvasSeg.addEventListener('mousedown', e=>{ 
    panSeg.active = true; 
    panSeg.last = {x:e.clientX, y:e.clientY}; 
    canvasSeg.style.cursor = 'grabbing'; 
});
window.addEventListener('mouseup', ()=>{ 
    panSeg.active = false; 
    panSeg.last = null; 
    canvasSeg.style.cursor = 'grab'; 
});
window.addEventListener('mousemove', e=>{
    if(!panSeg.active || !panSeg.last) return;
    const dx = e.clientX - panSeg.last.x, dy = e.clientY - panSeg.last.y;
    panSeg.last = {x:e.clientX, y:e.clientY};
    viewSeg.x -= dx / viewSeg.scale;
    viewSeg.y += dy / viewSeg.scale;
    renderSegments();
});
canvasSeg.addEventListener('wheel', e=>{
    e.preventDefault();
    const rect = canvasSeg.getBoundingClientRect();
    const mx = e.clientX - rect.left, my = e.clientY - rect.top;
    const before = screenToWorld(viewSeg, canvasSeg, mx, my);
    const delta = e.deltaY < 0 ? 1.12 : 0.88;
    viewSeg.scale = Math.max(20, Math.min(600, viewSeg.scale * delta));
    const after = screenToWorld(viewSeg, canvasSeg, mx, my);
    viewSeg.x += before.x - after.x; viewSeg.y += before.y - after.y;
    renderSegments();
}, { passive:false });

canvasPoly.addEventListener('mousedown', e=>{ 
    panPoly.active = true; 
    panPoly.last = {x:e.clientX, y:e.clientY}; 
    canvasPoly.style.cursor = 'grabbing'; 
});
window.addEventListener('mouseup', ()=>{ 
    panPoly.active = false; 
    panPoly.last = null; 
    canvasPoly.style.cursor = 'grab'; 
});
window.addEventListener('mousemove', e=>{
    if(!panPoly.active || !panPoly.last) return;
    const dx = e.clientX - panPoly.last.x, dy = e.clientY - panPoly.last.y;
    panPoly.last = {x:e.clientX, y:e.clientY};
    viewPoly.x -= dx / viewPoly.scale;
    viewPoly.y += dy / viewPoly.scale;
    renderPolygon();
});
canvasPoly.addEventListener('wheel', e=>{
    e.preventDefault();
    const rect = canvasPoly.getBoundingClientRect();
    const mx = e.clientX - rect.left, my = e.clientY - rect.top;
    const before = screenToWorld(viewPoly, canvasPoly, mx, my);
    const delta = e.deltaY < 0 ? 1.12 : 0.88;
    viewPoly.scale = Math.max(20, Math.min(600, viewPoly.scale * delta));
    const after = screenToWorld(viewPoly, canvasPoly, mx, my);
    viewPoly.x += before.x - after.x; viewPoly.y += before.y - after.y;
    renderPolygon();
}, { passive:false });

const sampleSegments = `4
3 5 7 7
1 1 4 4
6 0 6 8
2 2 8 6
2 2 6 6`;
inputSeg.value = sampleSegments;

const samplePolygon = `6
-8 0
-4 6
2 8
6 4
4 -4
-2 -6
5
-5 -5
5 -5
8 2
0 8
-8 2`;
inputPoly.value = samplePolygon;


function init(){
    try{ parseSegmentsText(sampleSegments); } catch(e){ }
    try{ parsePolygonText(samplePolygon); } catch(e){ }
    resizeCanvases();
    renderSegments();
    renderPolygon();
}
window.addEventListener('resize', ()=>{ resizeCanvases(); renderSegments(); renderPolygon(); });

function resizeCanvases(){
    const cards = document.querySelectorAll('.canvas-card');
    cards.forEach(card=>{
        const canvas = card.querySelector('canvas');
        const container = card.querySelector('.canvas-container');
        const w = container.clientWidth;
        const h = container.clientHeight;
        canvas.width = w;
        canvas.height = h;
    });
}
init();