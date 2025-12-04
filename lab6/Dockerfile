FROM nginx:alpine

# Копируем все файлы проекта (index.html, style.css, app.js и т.д.)
COPY . /usr/share/nginx/html/

# Открываем порт 80
EXPOSE 80

# Запускаем nginx в foreground
CMD ["nginx", "-g", "daemon off;"]
