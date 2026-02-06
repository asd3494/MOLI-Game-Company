(function() {
    'use strict';

    function clickTriangleEffect() {
        let tris = []; 
        let dots = []; 
        let cvs, ctx; 
        let w, h;
        const blueColors = ["#1E90FF", "#4169E1", "#00BFFF", "#87CEFA", "#6495ED", "#0099FF", "#7B68EE"];
        const triTypes = ["acute", "right", "obtuse", "isosceles", "equilateral"];
        const MAX_TRACK = 10;
        const DOT_TRACK = 5;
        let pressStart = 0;
        let isPress = false;
        let px = 0, py = 0;
        const LONG_PRESS = 800;
        // 进阶特效上限
        const MAX_SIZE = 20;
        const MAX_SPEED = 15;
        const MIN_ALPHA_SPEED = 8;

        const BASE_COUNT = 12;
        const MAX_COUNT = 30;
        const SCALE_DUR = 3000;

        const DOT_R_MIN = 1;
        const DOT_R_MAX = 3;
        const DOT_INTERVAL = 150;
        const DOT_INIT_SPD = 1.2;
        const DOT_ACCEL = 0.2;
        const DOT_ALPHA_SPD = 0.02;
        const DOT_FADE_BASE = 0.03;
        const DOT_SPAWN_R_MIN = 180;
        const DOT_SPAWN_R_MAX = 280;
        const SPEED_ALPHA_FACTOR = 0.0001;
        let lastDotSpawn = 0;

        cvs = document.createElement("canvas");
        document.body.appendChild(cvs);
        cvs.setAttribute("style", "width: 100%; height: 100%; top: 0; left: 0; z-index: 99999; position: fixed; pointer-events: none;");

        if (cvs.getContext && window.addEventListener) {
            ctx = cvs.getContext("2d");
            updateCvsSize();
            window.addEventListener('resize', updateCvsSize, false);
            requestAnimationFrame(loop);

            window.addEventListener("mousedown", function(e) {
                isPress = true;
                pressStart = Date.now();
                px = e.clientX;
                py = e.clientY;
                lastDotSpawn = Date.now();
            }, false);
            window.addEventListener("mouseup", createEffect, false);
            window.addEventListener("mouseout", function() {
                isPress = false;
                pressStart = 0;
                dots.forEach(dot => dot.needFade = true);
            }, false);

        } else {
            console.log("当前浏览器不支持Canvas或addEventListener！");
        }
        function updateCvsSize() {
            cvs.width = window.innerWidth * 2;
            cvs.height = window.innerHeight * 2;
            cvs.style.width = window.innerWidth + 'px';
            cvs.style.height = window.innerHeight + 'px';
            ctx.scale(2, 2);
            w = window.innerWidth;
            h = window.innerHeight;
        }
        function alphaToHex(alpha) {
            const hex = Math.floor(Math.max(0, Math.min(255, alpha * 255))).toString(16);
            return hex.length === 1 ? '0' + hex : hex;
        }
        function randInt(min, max) {
            return Math.floor(Math.random() * (max - min + 1)) + min;
        }
        function randFloat(min, max) {
            return Math.random() * (max - min) + min;
        }

        function createEffect() {
            if (!isPress) return;
            const pressDur = Date.now() - pressStart;
            isPress = false;
            pressStart = 0;
            dots.forEach(dot => dot.needFade = true);

            let sizeMin = 7, sizeMax = 13;
            let spdMin = 4, spdMax = 9;
            let alphaSpdMin = 25, alphaSpdMax = 35;
            let createCnt = BASE_COUNT;

            if (pressDur >= LONG_PRESS) {
                const overThresh = pressDur - LONG_PRESS;
                const scale = Math.min(1, overThresh / SCALE_DUR);
                createCnt = Math.floor(BASE_COUNT + scale * (MAX_COUNT - BASE_COUNT));
                sizeMax = 13 + scale * (MAX_SIZE - 13);
                spdMax = 9 + scale * (MAX_SPEED - 9);
                alphaSpdMin = 25 - scale * (25 - MIN_ALPHA_SPEED);
                alphaSpdMax = 35 - scale * (35 - MIN_ALPHA_SPEED);
            }

            for (let i = 0; i < createCnt; i++) {
                const triSpd = randInt(spdMin, spdMax);
                let triAlphaSpd = randInt(alphaSpdMin, alphaSpdMax);
                triAlphaSpd = triAlphaSpd + triSpd * 2;
                triAlphaSpd = Math.min(triAlphaSpd, 50);

                tris.push(new Triangle(
                    px, py,
                    randInt(sizeMin, sizeMax),
                    triSpd,
                    triAlphaSpd / 1000
                ));
            }
        }

        class Dot {
            constructor(tx, ty) {
                this.tx = tx;
                this.ty = ty;
                const r = randFloat(DOT_SPAWN_R_MIN, DOT_SPAWN_R_MAX);
                const angle = randFloat(0, Math.PI * 2);
                this.x = tx + Math.cos(angle) * r;
                this.y = ty + Math.sin(angle) * r;
                this.r = randFloat(DOT_R_MIN, DOT_R_MAX);
                this.color = blueColors[Math.floor(Math.random() * blueColors.length)];
                this.alpha = 0;
                this.spd = DOT_INIT_SPD;
                this.accel = DOT_ACCEL;
                this.needFade = false;
                this.angle = Math.atan2(ty - this.y, tx - this.x);
                this.track = [];
                this.arrived = false;
            }

            addTrack() {
                if (this.arrived) return;
                this.track.push({ x: this.x, y: this.y, alpha: this.alpha, r: this.r });
                if (this.track.length > DOT_TRACK) this.track.shift();
            }

            update() {
                if (this.arrived) {
                    this.alpha = 0;
                    return;
                }
                this.addTrack();

                const spdFactor = 1 + this.spd * SPEED_ALPHA_FACTOR;
                if (this.needFade) {
                    this.alpha = Math.max(0, this.alpha - DOT_FADE_BASE * spdFactor);
                } else {
                    this.alpha = Math.min(1, this.alpha + DOT_ALPHA_SPD * spdFactor);
                }

                const dx = this.tx - this.x;
                const dy = this.ty - this.y;
                const dist = Math.sqrt(dx * dx + dy * dy);
                const nextStep = this.spd + this.accel;

                if (dist <= this.r * 3 || nextStep >= dist) {
                    this.arrived = true;
                    this.alpha = 0;
                    this.track = [];
                    this.x = this.tx;
                    this.y = this.ty;
                } else {
                    this.spd += this.accel;
                    this.spd = Math.min(this.spd, 8);
                    this.x += Math.cos(this.angle) * this.spd;
                    this.y += Math.sin(this.angle) * this.spd;
                }
            }

            drawTrail() {
                if (this.track.length < 2 || this.alpha <= 0 || this.arrived) return;
                ctx.save();
                ctx.beginPath();
                ctx.moveTo(this.track[0].x, this.track[0].y);
                for (let i = 1; i < this.track.length; i++) {
                    ctx.lineTo(this.track[i].x, this.track[i].y);
                }
                const lineW = this.r * 0.3;
                if (lineW <= 0) return;
                const grad = ctx.createLinearGradient(
                    this.track[0].x, this.track[0].y,
                    this.track[this.track.length-1].x, this.track[this.track.length-1].y
                );
                grad.addColorStop(0, `${this.color}1A`);
                grad.addColorStop(1, `${this.color}${alphaToHex(Math.min(0.6, this.alpha))}`);
                ctx.lineWidth = lineW;
                ctx.lineCap = "butt";
                ctx.lineJoin = "miter";
                ctx.strokeStyle = grad;
                ctx.stroke();
                ctx.restore();
            }

            draw() {
                if (this.alpha <= 0 || this.arrived) return;
                this.drawTrail();
                ctx.save();
                ctx.globalAlpha = this.alpha;
                ctx.fillStyle = this.color;
                ctx.beginPath();
                ctx.arc(this.x, this.y, this.r, 0, Math.PI * 2);
                ctx.closePath();
                ctx.fill();
                ctx.restore();
            }
        }

        class Triangle {
            constructor(x, y, size, spd, alphaSpd) {
                this.x = x;
                this.y = y;
                this.angle = Math.random() * Math.PI * 2;
                this.spd = spd;
                this.rotSpd = (Math.random() - 0.5) * 18;
                this.rot = 0;
                this.size = size;
                this.color = blueColors[Math.floor(Math.random() * blueColors.length)];
                this.type = triTypes[Math.floor(Math.random() * triTypes.length)];
                this.alpha = 1;
                this.alphaSpd = alphaSpd;
                this.sizeSpd = randInt(5, 12) / 100;
                this.track = [];
            }

            update() {
                this.addTrack();
                this.x += Math.cos(this.angle) * this.spd;
                this.y += Math.sin(this.angle) * this.spd;
                this.rot += this.rotSpd;
                this.size = Math.max(1, this.size - this.sizeSpd);
                this.alpha = Math.max(0, this.alpha - this.alphaSpd);
            }

            addTrack() {
                this.track.push({ x: this.x, y: this.y, size: this.size, alpha: this.alpha });
                if (this.track.length > MAX_TRACK) this.track.shift();
            }

            drawSingle(ctx, x, y, rot, size, alpha) {
                if (alpha <= 0 || size <= 0) return;
                ctx.save();
                ctx.globalAlpha = alpha;
                ctx.translate(x, y);
                ctx.rotate(rot * Math.PI / 180);
                ctx.fillStyle = this.color;
                ctx.beginPath();
                switch (this.type) {
                    case "acute":
                        ctx.moveTo(0, -size);
                        ctx.lineTo(size * 0.7, size * 0.6);
                        ctx.lineTo(-size * 0.7, size * 0.6);
                        break;
                    case "right":
                        ctx.moveTo(0, -size * 0.8);
                        ctx.lineTo(size * 0.8, size * 0.8);
                        ctx.lineTo(-size * 0.8, size * 0.8);
                        break;
                    case "obtuse":
                        ctx.moveTo(0, -size);
                        ctx.lineTo(size * 1.0, size * 0.4);
                        ctx.lineTo(-size * 1.0, size * 0.4);
                        break;
                    case "isosceles":
                        ctx.moveTo(0, -size);
                        ctx.lineTo(size * 0.6, size * 0.8);
                        ctx.lineTo(-size * 0.6, size * 0.8);
                        break;
                    case "equilateral":
                        const h = size * Math.sqrt(3) / 2;
                        ctx.moveTo(0, -h / 2);
                        ctx.lineTo(size / 2, h / 2);
                        ctx.lineTo(-size / 2, h / 2);
                        break;
                }
                ctx.closePath();
                ctx.fill();
                ctx.restore();
            }
            drawTrail(ctx) {
                if (this.track.length < 2 || this.alpha <= 0) return;

                const trackCnt = this.track.length;
                const startP = this.track[0];
                const endP = this.track[trackCnt - 1];
                const trailAngle = Math.atan2(endP.y - startP.y, endP.x - startP.x);
                const trailLen = Math.sqrt(
                    Math.pow(endP.x - startP.x, 2) +
                    Math.pow(endP.y - startP.y, 2)
                );
                const endW = endP.size * 0.3; 
                const startW = endW * 0.05; 

                ctx.save();
                const endLX = endP.x + Math.cos(trailAngle + Math.PI / 2) * endW;
                const endLY = endP.y + Math.sin(trailAngle + Math.PI / 2) * endW;
                const endRX = endP.x + Math.cos(trailAngle - Math.PI / 2) * endW;
                const endRY = endP.y + Math.sin(trailAngle - Math.PI / 2) * endW;
                const startX = startP.x + Math.cos(trailAngle) * startW;
                const startY = startP.y + Math.sin(trailAngle) * startW;

                ctx.beginPath();
                ctx.moveTo(endLX, endLY);
                ctx.lineTo(endRX, endRY);
                ctx.lineTo(startX, startY);
                ctx.closePath();

                const grad = ctx.createLinearGradient(
                    endP.x, endP.y,
                    startP.x, startP.y
                );
                grad.addColorStop(0, `${this.color}${alphaToHex(Math.min(0.4, this.alpha))}`);
                grad.addColorStop(1, `${this.color}10`);
                ctx.fillStyle = grad;
                ctx.fill();
                ctx.restore();
            }

            draw() {
                if (this.alpha <= 0) return;
                this.drawTrail(ctx);
                this.drawSingle(ctx, this.x, this.y, this.rot, this.size, this.alpha);
            }
        }

        function loop() {
            ctx.clearRect(0, 0, w, h);
            if (isPress) {
                const pressDur = Date.now() - pressStart;
                if (pressDur >= LONG_PRESS) {
                    const now = Date.now();
                    if (now - lastDotSpawn >= DOT_INTERVAL) {
                        dots.push(new Dot(px, py));
                        lastDotSpawn = now;
                    }
                }
            }
            // 圆点更新绘制
            dots.forEach(dot => dot.update());
            dots.forEach(dot => dot.draw());
            dots = dots.filter(dot => dot.alpha > 0 && !dot.arrived);
            tris.forEach(tri => tri.update());
            tris.forEach(tri => tri.draw());
            tris = tris.filter(tri => tri.alpha > 0);

            requestAnimationFrame(loop);
        }
    }

    clickTriangleEffect();
})();
