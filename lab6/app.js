/*const I4 = () => [1,0,0,0,  0,1,0,0,  0,0,1,0,  0,0,0,1];

function mul4(A, B) {
  const C = new Array(16).fill(0);
  for (let r=0;r<4;r++) for (let c=0;c<4;c++) for (let k=0;k<4;k++)
    C[r*4+c] += A[r*4+k]*B[k*4+c];
  return C;
}
function applyMat(M, [x,y,z]) {
  const xp = M[0]*x + M[1]*y + M[2]*z + M[3];
  const yp = M[4]*x + M[5]*y + M[6]*z + M[7];
  const zp = M[8]*x + M[9]*y + M[10]*z + M[11];
  return [xp, yp, zp];
}
function scaleMat(sx,sy,sz){ return [sx,0,0,0,  0,sy,0,0,  0,0,sz,0,  0,0,0,1]; }
function translateMat(tx,ty,tz){ return [1,0,0,tx,  0,1,0,ty,  0,0,1,tz,  0,0,0,1]; }
function rotateAxisUnitMat(ux,uy,uz,deg){
  const a = deg*Math.PI/180, c=Math.cos(a), s=Math.sin(a), t=1-c;
  return [
    t*ux*ux + c,     t*ux*uy - s*uz, t*ux*uz + s*uy, 0,
    t*uy*ux + s*uz,  t*uy*uy + c,    t*uy*uz - s*ux, 0,
    t*uz*ux - s*uy,  t*uz*uy + s*ux, t*uz*uz + c,    0,
    0,0,0,1
  ];
}
function formatMatrix(M){
  const rows = [
    [M[0],M[1],M[2],M[3]],
    [M[4],M[5],M[6],M[7]],
    [M[8],M[9],M[10],M[11]],
    [M[12],M[13],M[14],M[15]],
  ];
  return rows.map(r => r.map(v => (Math.abs(v)<1e-10?0:+v.toFixed(5))).join('\t')).join('\n');
}

// ---------- Геометрия буквы G ----------
function buildGoogleStyleG({
  R_outer = 1.0,
  R_inner = 0.6,
  gapStartDeg = 0,    // начало выреза (3 часа)
  gapEndDeg   = 53,   // конец выреза (1:30)
  thickness = 0.35,
  segments = 180,
  barHeight = 0.40,   // ширина перемычки (перпендикуляр к радиусу)
  barDepth  = 0.35,   // толщина перемычки по Z
  barOvershoot = 0.15,// выступ за края
  stretchX = 1.3,     // коэффициент вытянутости по X
  stretchY = 1.0      // коэффициент вытянутости по Y
} = {}) {

  const zFront = +thickness/2;
  const zBack  = -thickness/2;

  // переводим градусы в радианы
  const angStart = gapEndDeg * Math.PI/180;
  const angEnd   = (360 + gapStartDeg) * Math.PI/180;

  // внешняя дуга (с учётом stretchX/stretchY)
  const outer = [];
  for (let i=0;i<=segments;i++){
    const t = i/segments;
    const a = angStart + t*(angEnd-angStart);
    outer.push({x: stretchX*R_outer*Math.cos(a), y: stretchY*R_outer*Math.sin(a)});
  }

  // внутренняя дуга (с учётом stretchX/stretchY)
  const inner = [];
  for (let i=0;i<=segments;i++){
    const t = i/segments;
    const a = angEnd - t*(angEnd-angStart);
    inner.push({x: stretchX*R_inner*Math.cos(a), y: stretchY*R_inner*Math.sin(a)});
  }

  const G2D = [...outer, ...inner];
  const verticesFront = G2D.map(p => [p.x, p.y, zFront]);
  const verticesBack  = G2D.map(p => [p.x, p.y, zBack]);

  let vertices = [...verticesFront, ...verticesBack];
  const nRing = G2D.length, F = i => i, B = i => nRing+i;
  const edges = [];

  // рёбра кольца
  for (let i=0;i<outer.length-1;i++) edges.push([F(i),F(i+1)]);
  for (let i=outer.length;i<G2D.length-1;i++) edges.push([F(i),F(i+1)]);
  edges.push([F(0),F(G2D.length-1)]);
  edges.push([F(outer.length-1),F(outer.length)]);
  for (let i=0;i<outer.length-1;i++) edges.push([B(i),B(i+1)]);
  for (let i=outer.length;i<G2D.length-1;i++) edges.push([B(i),B(i+1)]);
  edges.push([B(0),B(G2D.length-1)]);
  edges.push([B(outer.length-1),B(outer.length)]);
  for (let i=0;i<nRing;i++) edges.push([F(i),B(i)]);

  // ---------- Перемычка ----------
  const ang = gapStartDeg * Math.PI/180;
  const dx = Math.cos(ang);
  const dy = Math.sin(ang);

  const startR = R_inner - barOvershoot;
  const endR   = R_outer + barOvershoot;

  const halfD = barDepth/2;
  const lower = 0;             // центр
  const upper = barHeight/2;   // верхняя половина

  const nx = -dy;
  const ny = dx;

  const sx = stretchX*startR * dx;
  const sy = stretchY*startR * dy;
  const ex = stretchX*endR * dx;
  const ey = stretchY*endR * dy;

  const barVerts = [
    [sx - lower*nx, sy - lower*ny, -halfD],
    [ex - lower*nx, ey - lower*ny, -halfD],
    [ex + upper*nx, ey + upper*ny, -halfD],
    [sx + upper*nx, sy + upper*ny, -halfD],
    [sx - lower*nx, sy - lower*ny, +halfD],
    [ex - lower*nx, ey - lower*ny, +halfD],
    [ex + upper*nx, ey + upper*ny, +halfD],
    [sx + upper*nx, sy + upper*ny, +halfD],
  ];

  const barOffset = vertices.length;
  vertices = vertices.concat(barVerts);

  const boxEdges = [[0,1],[1,2],[2,3],[3,0],[4,5],[5,6],[6,7],[7,4],[0,4],[1,5],[2,6],[3,7]];
  for (const [i,j] of boxEdges) edges.push([barOffset+i, barOffset+j]);

  return { vertices, edges };
}



// ---------- Сцена ----------
const canvas3d=document.getElementById('canvas3d'),ctx3d=canvas3d.getContext('2d');
const canvasOxy=document.getElementById('canvasOxy'),ctxOxy=canvasOxy.getContext('2d');
const canvasOxz=document.getElementById('canvasOxz'),ctxOxz=canvasOxz.getContext('2d');
const canvasOyz=document.getElementById('canvasOyz'),ctxOyz=canvasOyz.getContext('2d');

let focal=900, zoom=200, M=I4();
const {vertices,edges}=buildGoogleStyleG({});

// ---------- Проекции ----------
function worldToScreen([x,y,z],canvas){
  const denom=(focal-z);
  const sx=(x*focal/denom)*(zoom/100);
  const sy=(y*focal/denom)*(zoom/100);
  return [canvas.width/2+sx,canvas.height/2-sy];
}
function orthoXY([x,y,z],canvas){const s=zoom/100;return[canvas.width/2+x*s,canvas.height/2-y*s];}
function orthoXZ([x,y,z],canvas){const s=zoom/100;return[canvas.width/2+x*s,canvas.height/2-z*s];}
function orthoYZ([x,y,z],canvas){const s=zoom/100;return[canvas.width/2+z*s,canvas.height/2-y*s];}

// ---------- Рисование ----------
function drawAxes(ctx,canvas,proj){
  ctx.lineWidth=1.6;
  const o=[0,0,0],X=[1.8,0,0],Y=[0,1.8,0],Z=[0,0,1.8];
  const oS=proj(o,canvas),xS=proj(X,canvas),yS=proj(Y,canvas),zS=proj(Z,canvas);
  ctx.strokeStyle='#ff5252';ctx.beginPath();ctx.moveTo(...oS);ctx.lineTo(...xS);ctx.stroke();
  ctx.strokeStyle='#69f0ae'; ctx.beginPath(); ctx.moveTo(...oS); ctx.lineTo(...yS); ctx.stroke();
  ctx.strokeStyle='#64b5f6';ctx.beginPath();ctx.moveTo(...oS);ctx.lineTo(...zS);ctx.stroke();
}

function drawWire(ctx,canvas,pts,proj){
  ctx.lineWidth=1.2;
  ctx.strokeStyle='#e8eaf6';
  for(const [i,j] of edges){
    const p=proj(pts[i],canvas);
    const q=proj(pts[j],canvas);
    ctx.beginPath();
    ctx.moveTo(p[0],p[1]);
    ctx.lineTo(q[0],q[1]);
    ctx.stroke();
  }
}

// ---------- Рендер ----------
function render(){
  const transformed = vertices.map(v => applyMat(M, v));

  // Основная сцена (перспектива)
  ctx3d.clearRect(0,0,canvas3d.width, canvas3d.height);
  drawAxes(ctx3d, canvas3d, worldToScreen);
  drawWire(ctx3d, canvas3d, transformed, worldToScreen);

  // Ортографические
  ctxOxy.clearRect(0,0,canvasOxy.width, canvasOxy.height);
  drawAxes(ctxOxy, canvasOxy, orthoXY);
  drawWire(ctxOxy, canvasOxy, transformed, orthoXY);

  ctxOxz.clearRect(0,0,canvasOxz.width, canvasOxz.height);
  drawAxes(ctxOxz, canvasOxz, orthoXZ);
  drawWire(ctxOxz, canvasOxz, transformed, orthoXZ);

  ctxOyz.clearRect(0,0,canvasOyz.width, canvasOyz.height);
  drawAxes(ctxOyz, canvasOyz, orthoYZ);
  drawWire(ctxOyz, canvasOyz, transformed, orthoYZ);

  document.getElementById('matrixOut').textContent = formatMatrix(M);
}

// ---------- Контролы ----------
document.getElementById('applyScale').addEventListener('click', () => {
  const sx = parseFloat(document.getElementById('sx').value || '1');
  const sy = parseFloat(document.getElementById('sy').value || '1');
  const sz = parseFloat(document.getElementById('sz').value || '1');
  M = mul4(scaleMat(sx,sy,sz), M);
  render();
});

document.getElementById('applyTranslate').addEventListener('click', () => {
  const tx = parseFloat(document.getElementById('tx').value || '0');
  const ty = parseFloat(document.getElementById('ty').value || '0');
  const tz = parseFloat(document.getElementById('tz').value || '0');
  M = mul4(translateMat(tx,ty,tz), M);
  render();
});

document.getElementById('applyRotate').addEventListener('click', () => {
  const ux = parseFloat(document.getElementById('ux').value || '0');
  const uy = parseFloat(document.getElementById('uy').value || '1');
  const uz = parseFloat(document.getElementById('uz').value || '0');
  const angle = parseFloat(document.getElementById('angle').value || '0');
  const R = rotateAxisUnitMat(ux,uy,uz,angle);
  M = mul4(R, M);
  render();
});

document.getElementById('reset').addEventListener('click', () => { M = I4(); render(); });
document.getElementById('fit').addEventListener('click', () => {
  focal = 900; zoom = 200;
  let cx=0,cy=0,cz=0;
  for (const p of vertices){ cx+=p[0]; cy+=p[1]; cz+=p[2]; }
  cx/=vertices.length; cy/=vertices.length; cz/=vertices.length;
  M = mul4(translateMat(-cx,-cy,-cz), M);
  render();
});

// ---------- Вращение мышью ----------
let isDragging = false;
let lastX, lastY;

canvas3d.addEventListener('mousedown', e => {
  isDragging = true;
  lastX = e.clientX;
  lastY = e.clientY;
});
canvas3d.addEventListener('mouseup', () => { isDragging = false; });
canvas3d.addEventListener('mouseleave', () => { isDragging = false; });
canvas3d.addEventListener('mousemove', e => {
  if (!isDragging) return;
  const dx = e.clientX - lastX;
  const dy = e.clientY - lastY;
  lastX = e.clientX;
  lastY = e.clientY;

  const rotY = rotateAxisUnitMat(0,1,0, dx * 0.5);
  const rotX = rotateAxisUnitMat(1,0,0, dy * 0.5);
  M = mul4(rotY, M);
  M = mul4(rotX, M);
  render();
});

// ---------- Инициализация ----------
render();
*/
// ---------- Линейная алгебра ----------
// ---------- Линейная алгебра ----------
const I4 = () => [1,0,0,0,  0,1,0,0,  0,0,1,0,  0,0,0,1];

function mul4(A, B) {
  const C = new Array(16).fill(0);
  for (let r=0;r<4;r++) for (let c=0;c<4;c++) for (let k=0;k<4;k++)
    C[r*4+c] += A[r*4+k]*B[k*4+c];
  return C;
}
function applyMat(M, [x,y,z]) {
  const xp = M[0]*x + M[1]*y + M[2]*z + M[3];
  const yp = M[4]*x + M[5]*y + M[6]*z + M[7];
  const zp = M[8]*x + M[9]*y + M[10]*z + M[11];
  return [xp, yp, zp];
}
function scaleMat(sx,sy,sz){ return [sx,0,0,0,  0,sy,0,0,  0,0,sz,0,  0,0,0,1]; }
function translateMat(tx,ty,tz){ return [1,0,0,tx,  0,1,0,ty,  0,0,1,tz,  0,0,0,1]; }
function rotateAxisUnitMat(ux,uy,uz,deg){
  const a = deg*Math.PI/180, c=Math.cos(a), s=Math.sin(a), t=1-c;
  return [
    t*ux*ux + c,     t*ux*uy - s*uz, t*ux*uz + s*uy, 0,
    t*uy*ux + s*uz,  t*uy*uy + c,    t*uy*uz - s*ux, 0,
    t*uz*ux - s*uy,  t*uz*uy + s*ux, t*uz*uz + c,    0,
    0,0,0,1
  ];
}
function formatMatrix(M){
  const rows = [
    [M[0],M[1],M[2],M[3]],
    [M[4],M[5],M[6],M[7]],
    [M[8],M[9],M[10],M[11]],
    [M[12],M[13],M[14],M[15]],
  ];
  return rows.map(r => r.map(v => (Math.abs(v)<1e-10?0:+v.toFixed(5))).join('\t')).join('\n');
}

// ---------- Геометрия буквы G ----------
function buildGoogleStyleG({
  R_outer = 1.2,
  R_inner = 0.7,
  gapStartDeg = 0,
  gapEndDeg   = 53,
  thickness = 0.35,
  segments = 180,
  barHeight = 0.40,
  barDepth  = 0.35,
  barOvershoot = 0.15,
  stretchX = 1.3,
  stretchY = 1.0
} = {}) {
  const zFront = +thickness/2;
  const zBack  = -thickness/2;

  const angStart = gapEndDeg * Math.PI/180;
  const angEnd   = (360 + gapStartDeg) * Math.PI/180;

  const outer = [];
  for (let i=0;i<=segments;i++){
    const t = i/segments;
    const a = angStart + t*(angEnd-angStart);
    outer.push({x: stretchX*R_outer*Math.cos(a), y: stretchY*R_outer*Math.sin(a)});
  }

  const inner = [];
  for (let i=0;i<=segments;i++){
    const t = i/segments;
    const a = angEnd - t*(angEnd-angStart);
    inner.push({x: stretchX*R_inner*Math.cos(a), y: stretchY*R_inner*Math.sin(a)});
  }

  const G2D = [...outer, ...inner];
  const verticesFront = G2D.map(p => [p.x, p.y, zFront]);
  const verticesBack  = G2D.map(p => [p.x, p.y, zBack]);

  let vertices = [...verticesFront, ...verticesBack];
  const nRing = G2D.length, F = i => i, B = i => nRing+i;
  const edges = [];

  for (let i=0;i<outer.length-1;i++) edges.push([F(i),F(i+1)]);
  for (let i=outer.length;i<G2D.length-1;i++) edges.push([F(i),F(i+1)]);
  edges.push([F(0),F(G2D.length-1)]);
  edges.push([F(outer.length-1),F(outer.length)]);
  for (let i=0;i<outer.length-1;i++) edges.push([B(i),B(i+1)]);
  for (let i=outer.length;i<G2D.length-1;i++) edges.push([B(i),B(i+1)]);
  edges.push([B(0),B(G2D.length-1)]);
  edges.push([B(outer.length-1),B(outer.length)]);
  for (let i=0;i<nRing;i++) edges.push([F(i),B(i)]);

  const ang = gapStartDeg * Math.PI/180;
  const dx = Math.cos(ang), dy = Math.sin(ang);
  const startR = R_inner - barOvershoot;
  const endR   = R_outer + barOvershoot;
  const halfD = barDepth/2;
  const lower = 0, upper = barHeight/2;
  const nx = -dy, ny = dx;
  const sx = stretchX*startR * dx, sy = stretchY*startR * dy;
  const ex = stretchX*endR * dx,   ey = stretchY*endR * dy;

  const barVerts = [
    [sx - lower*nx, sy - lower*ny, -halfD],
    [ex - lower*nx, ey - lower*ny, -halfD],
    [ex + upper*nx, ey + upper*ny, -halfD],
    [sx + upper*nx, sy + upper*ny, -halfD],
    [sx - lower*nx, sy - lower*ny, +halfD],
    [ex - lower*nx, ey - lower*ny, +halfD],
    [ex + upper*nx, ey + upper*ny, +halfD],
    [sx + upper*nx, sy + upper*ny, +halfD],
  ];

  const barOffset = vertices.length;
  vertices = vertices.concat(barVerts);

  const boxEdges = [[0,1],[1,2],[2,3],[3,0],[4,5],[5,6],[6,7],[7,4],[0,4],[1,5],[2,6],[3,7]];
  for (const [i,j] of boxEdges) edges.push([barOffset+i, barOffset+j]);

  return { vertices, edges };
}

// ---------- Сцена и масштаб ----------
const canvas3d=document.getElementById('canvas3d'),ctx3d=canvas3d.getContext('2d');
const canvasOxy=document.getElementById('canvasOxy'),ctxOxy=canvasOxy.getContext('2d');
const canvasOxz=document.getElementById('canvasOxz'),ctxOxz=canvasOxz.getContext('2d');
const canvasOyz=document.getElementById('canvasOyz'),ctxOyz=canvasOyz.getContext('2d');

let focal=900, zoom=250, M=I4();
const {vertices,edges}=buildGoogleStyleG({});

function getScale(){
  const zoomVal = parseFloat(document.getElementById("zoom").value || zoom);
  const baseSize = 3.2;
  const maxZoom = parseFloat(document.getElementById("zoom").max || "400");
  const targetPixels = canvas3d.width * 0.8;
  const scaleAtMax = targetPixels / baseSize;
  return (zoomVal / maxZoom) * scaleAtMax;
}

// ---------- Проекции ----------
function worldToScreen([x,y,z],canvas){
  const denom=(focal-z);
  const s=getScale();
  const sx=(x*focal/denom)*s;
  const sy=(y*focal/denom)*s;
  return [canvas.width/2+sx,canvas.height/2-sy];
}
function orthoXY([x,y,z],canvas){
  const s = getScale();
  return [canvas.width/2 + x*s, canvas.height/2 - y*s];
}
function orthoXZ([x,y,z],canvas){
  const s = getScale();
  return [canvas.width/2 + x*s, canvas.height/2 - z*s];
}
function orthoYZ([x,y,z],canvas){
  const s = getScale();
  return [canvas.width/2 + z*s, canvas.height/2 - y*s];
}

// ---------- Рисование ----------
function drawAxes(ctx,canvas,proj){
  ctx.lineWidth = 2.0;
  const maxX = canvas.width / 100;
  const maxY = canvas.height / 100;
  const maxZ = canvas.height / 100;

  const o = [0,0,0];
  const X = [ maxX, 0, 0];
  const Y = [ 0, maxY, 0];
  const Z = [ -maxZ, -maxZ, -maxZ]; // ось Z диагонально

  const oS = proj(o,canvas),
        xS = proj(X,canvas),
        yS = proj(Y,canvas),
        zS = proj(Z,canvas);

  ctx.strokeStyle='#ff5252'; ctx.beginPath(); ctx.moveTo(...oS); ctx.lineTo(...xS); ctx.stroke();
  ctx.strokeStyle='#69f0ae'; ctx.beginPath(); ctx.moveTo(...oS); ctx.lineTo(...yS); ctx.stroke();
  ctx.strokeStyle='#64b5f6'; ctx.beginPath(); ctx.moveTo(...oS); ctx.lineTo(...zS); ctx.stroke();

  ctx.fillStyle='#ffff00';
  ctx.font="14px sans-serif";
  ctx.fillText("X", xS[0]+5, xS[1]);
  ctx.fillText("Y", yS[0]+5, yS[1]);
  ctx.fillText("Z", zS[0]+5, zS[1]);
}


function drawWire(ctx,canvas,pts,proj){
  ctx.lineWidth=1.2;
  ctx.strokeStyle='#e8eaf6';
  for(const [i,j] of edges){
    const p=proj(pts[i],canvas);
    const q=proj(pts[j],canvas);
    ctx.beginPath();
    ctx.moveTo(p[0],p[1]);
    ctx.lineTo(q[0],q[1]);
    ctx.stroke();
  }
}

// ---------- Рендер ----------
function render(){
  const transformed = vertices.map(v => applyMat(M, v));

  ctx3d.clearRect(0,0,canvas3d.width, canvas3d.height);
  drawAxes(ctx3d, canvas3d, worldToScreen);
  drawWire(ctx3d, canvas3d, transformed, worldToScreen);

  ctxOxy.clearRect(0,0,canvasOxy.width, canvasOxy.height);
  drawAxes(ctxOxy, canvasOxy, orthoXY);
  drawWire(ctxOxy, canvasOxy, transformed, orthoXY);

  ctxOxz.clearRect(0,0,canvasOxz.width, canvasOxz.height);
  drawAxes(ctxOxz, canvasOxz, orthoXZ);
  drawWire(ctxOxz, canvasOxz, transformed, orthoXZ);

  ctxOyz.clearRect(0,0,canvasOyz.width, canvasOyz.height);
  drawAxes(ctxOyz, canvasOyz, orthoYZ);
  drawWire(ctxOyz, canvasOyz, transformed, orthoYZ);

  document.getElementById('matrixOut').textContent = formatMatrix(M);
}

// ---------- Контролы ----------
document.getElementById('applyScale').addEventListener('click', () => {
  const sx = parseFloat(document.getElementById('sx').value || '1');
  const sy = parseFloat(document.getElementById('sy').value || '1');
  const sz = parseFloat(document.getElementById('sz').value || '1');
  M = mul4(scaleMat(sx,sy,sz), M);
  render();
});

document.getElementById('applyTranslate').addEventListener('click', () => {
  const tx = parseFloat(document.getElementById('tx').value || '0');
  const ty = parseFloat(document.getElementById('ty').value || '0');
  const tz = parseFloat(document.getElementById('tz').value || '0');
  M = mul4(translateMat(tx,ty,tz), M);
  render();
});

document.getElementById('applyRotate').addEventListener('click', () => {
  const ux = parseFloat(document.getElementById('ux').value || '0');
  const uy = parseFloat(document.getElementById('uy').value || '1');
  const uz = parseFloat(document.getElementById('uz').value || '0');
  const angle = parseFloat(document.getElementById('angle').value || '0');
  const R = rotateAxisUnitMat(ux,uy,uz,angle);
  M = mul4(R, M);
  render();
});

document.getElementById('reset').addEventListener('click', () => { M = I4(); render(); });
document.getElementById('fit').addEventListener('click', () => {
  focal = 900;
  let cx=0,cy=0,cz=0;
  for (const p of vertices){ cx+=p[0]; cy+=p[1]; cz+=p[2]; }
  cx/=vertices.length; cy/=vertices.length; cz/=vertices.length;
  M = mul4(translateMat(-cx,-cy,-cz), I4());
  render();
});

// ---------- Ползунки ----------
["rotX","rotY","rotZ","zoom"].forEach(id=>{
  document.getElementById(id).addEventListener("input", ()=>{
    zoom = parseFloat(document.getElementById("zoom").value);
    const gx=parseFloat(document.getElementById("rotX").value);
    const gy=parseFloat(document.getElementById("rotY").value);
    const gz=parseFloat(document.getElementById("rotZ").value);
    M = I4();
    M = mul4(rotateAxisUnitMat(1,0,0,gx), M);
    M = mul4(rotateAxisUnitMat(0,1,0,gy), M);
    M = mul4(rotateAxisUnitMat(0,0,1,gz), M);
    render();
  });
});

// ---------- Вращение мышью ----------
let isDragging = false;
let lastX, lastY;

canvas3d.addEventListener('mousedown', e => {
  isDragging = true;
  lastX = e.clientX;
  lastY = e.clientY;
});
canvas3d.addEventListener('mouseup', () => { isDragging = false; });
canvas3d.addEventListener('mouseleave', () => { isDragging = false; });
canvas3d.addEventListener('mousemove', e => {
  if (!isDragging) return;
  const dx = e.clientX - lastX;
  const dy = e.clientY - lastY;
  lastX = e.clientX;
  lastY = e.clientY;

  const rotY = rotateAxisUnitMat(0,1,0, dx * 0.5);
  const rotX = rotateAxisUnitMat(1,0,0, dy * 0.5);
  M = mul4(rotY, M);
  M = mul4(rotX, M);
  render();
});

// ---------- Инициализация ----------
render();
