let canvas = document.getElementById("canvas");
let ctx = canvas.getContext("2d");

let particleCountEl = document.getElementById("particleCount");
let updateCountEl = document.getElementById("updateCount");
let simTimeEl = document.getElementById("simTime");

const WIDTH = 200;
const HEIGHT = 100;
const INTERVAL_MS = 20;
const GRAVITY = 0.1;

const KIND_AIR = 0;
const KIND_SOLID = 1;
const KIND_WATER = 2;

let fields = {
	kinds: [],
	vx: [],
	moved: [],
};

let pendingSet = new Set();
let pendingUpdates = [];

let particles = [];

function initMatrix(mat, w, h) {
	for (let y = 0; y < h; ++y) {
		let row = mat[y] = [];
		for (let x = 0; x < w; ++x) {
			row.push(0);
		}
	}
}

for (let f of ["kinds", "vx", "moved"]) {
	initMatrix(fields[f], WIDTH, HEIGHT);
}

for (let y = 0; y < HEIGHT; ++y) {
	fields.kinds[y][0] = fields.kinds[y][WIDTH - 1] = KIND_SOLID;
}
for (let x = 0; x < WIDTH; ++x) {
	fields.kinds[0][x] = fields.kinds[HEIGHT - 1][x] = KIND_SOLID;
}

for (let y = 0; y < HEIGHT; ++y) {
	for (let x = 0; x < WIDTH; ++x) {
		pendingUpdates.push([x, y]);
	}
}

function shuffle(arr) {
	for (let len = arr.length; len; --len) {
		let i1 = len - 1;
		let i2 = Math.floor(Math.random() * len);
		let tmp = arr[i1];
		arr[i1] = arr[i2];
		arr[i2] = tmp;
	}
}

function addParticle(x, y, vx, vy) {
	particles.push({x, y: y, vx, vy});
}

function tryPlaceParticle(particle, x, y) {
	let place = null;
	if (fields.kinds[y][x] == KIND_AIR) {
		place = [y, x];
	} else if (x > 0 && fields.kinds[y][x - 1] == KIND_AIR) {
		place = [y, x - 1];
	} else if (x < WIDTH - 1 && fields.kinds[y][x + 1] == KIND_AIR) {
		place = [y, x + 1];
	} else if (y > 0 && fields.kinds[y - 1][x] == KIND_AIR) {
		place = [y - 1, x];
	} else if (y < HEIGHT - 1 && fields.kinds[y + 1][x] == KIND_AIR) {
		place = [y + 1, x];
	}

	if (place) {
		let [py, px] = place;
		fields.kinds[py][px] = KIND_WATER;
		triggerUpdateAround(px, py);
		return true;
	}

	particle.vx = 0;
	if (particle.vy > 0) {
		particle.vy = 0;
	}
	particle.vy -= GRAVITY * 1.1;
	return false;
}

function updateParticle(particle) {
	particle.vy += GRAVITY;

	let y = Math.round(particle.y);
	if (y < 0 || y >= HEIGHT) {
		return false;
	}

	let dx = particle.vx;
	while (Math.abs(dx) > 0.001) {
		let step = Math.min(dx, 1);
		particle.x += step;
		dx -= step;

		let x = Math.round(particle.x);
		if (x < 0 || x >= WIDTH) {
			return false;
		}

		if (fields.kinds[y][x] != KIND_AIR) {
			let sign = step > 0 ? 1 : -1;
			if (tryPlaceParticle(particle, x - sign, y)) {
				return false;
			} else {
				break;
			}
		}
	}

	let x = Math.round(particle.x);
	let dy = particle.vy;
	while (Math.abs(dy) > 0.001) {
		let step = Math.min(dy, 1);
		particle.y += step;
		dy -= step;

		let y = Math.round(particle.y);
		if (y < 0 || y >= HEIGHT) {
			return false;
		}

		if (fields.kinds[y][x] != KIND_AIR) {
			let sign = step > 0 ? 1 : -1;
			if (tryPlaceParticle(particle, x, y - sign)) {
				return false;
			} else {
				break;
			}
		}
	}

	return true;
}

function draw() {
	canvas.width = canvas.clientWidth * window.devicePixelRatio;
	canvas.height = canvas.clientHeight * window.devicePixelRatio;

	const pw = canvas.width / WIDTH;
	const ph = canvas.height / HEIGHT;

	for (let y = 0; y < HEIGHT; ++y) {
		for (let x = 0; x < WIDTH; ++x) {
			const kind = fields.kinds[y][x];
			if (kind == KIND_AIR) {
				continue;
			} else if (kind == KIND_SOLID) {
				ctx.fillStyle = "#000000";
			} else if (kind == KIND_WATER) {
				ctx.fillStyle = "#7777ff";
			}

			const px = x * pw;
			const py = y * ph;

			ctx.fillRect(px, py, pw, ph);
		}
	}

	ctx.fillStyle = "#7777ff";
	for (let {x, y} of particles) {
		ctx.fillRect(x * pw, y * ph, pw, ph);
	}

	ctx.fillStyle = "rgba(10, 255, 10, 0.2)";
	for (let [x, y] of pendingUpdates) {
		ctx.fillRect(x * pw, y * ph, pw, ph);
	}
}

function triggerUpdate(x, y) {
	let key = (y << 16) + x;
	if (pendingSet.has(key)) {
		return;
	}

	pendingUpdates.push([x, y]);
	pendingSet.add(key);
}

function triggerUpdateAround(x, y) {
	triggerUpdate(x, y);
	triggerUpdate(x + 1, y);
	triggerUpdate(x - 1, y);
	triggerUpdate(x, y + 1);
	triggerUpdate(x, y - 1);
}

let currentEvt = null;
function onMouse(evt) {
	currentEvt = evt;
	handleMouse();
	draw();
}

function handleMouse() {
	let evt = currentEvt;
	if (!evt || evt.buttons == 0) {
		return;
	}

	let x = evt.clientX - canvas.offsetLeft;
	let y = evt.clientY - canvas.offsetTop;
	let cellX = Math.floor(x / canvas.clientWidth * WIDTH)
	let cellY = Math.floor(y / canvas.clientHeight * HEIGHT)

	if (cellX < 2) cellX = 2;
	if (cellX > WIDTH - 3) cellX = WIDTH - 3;
	if (cellY < 2) cellY = 2;
	if (cellY > HEIGHT - 3) cellY = HEIGHT - 3;

	let kind;
	if (evt.shiftKey) {
		kind = KIND_AIR;
	} else if (evt.buttons == 1) {
		kind = KIND_SOLID;
	} else {
		kind = KIND_WATER;
	}

	fields.kinds[cellY - 1][cellX - 1] = kind;
	fields.kinds[cellY - 1][cellX] = kind;
	fields.kinds[cellY - 1][cellX + 1] = kind;
	fields.kinds[cellY][cellX - 1] = kind;
	fields.kinds[cellY][cellX] = kind;
	fields.kinds[cellY][cellX + 1] = kind;
	fields.kinds[cellY + 1][cellX - 1] = kind;
	fields.kinds[cellY + 1][cellX] = kind;
	fields.kinds[cellY + 1][cellX + 1] = kind;

	triggerUpdateAround(cellX - 1, cellY - 1);
	triggerUpdateAround(cellX - 1, cellY);
	triggerUpdateAround(cellX - 1, cellY + 1);
	triggerUpdateAround(cellX, cellY - 1);
	triggerUpdateAround(cellX, cellY);
	triggerUpdateAround(cellX, cellY + 1);
	triggerUpdateAround(cellX + 1, cellY - 1);
	triggerUpdateAround(cellX + 1, cellY);
	triggerUpdateAround(cellX + 1, cellY + 1);
}

canvas.addEventListener("mousedown", onMouse);
canvas.addEventListener("mouseup", onMouse);
canvas.addEventListener("mousemove", onMouse);
canvas.addEventListener("contextmenu", evt => evt.preventDefault());

function applyRules(x, y) {
	if (fields.moved[y][x]) {
		return;
	}

	let kind = fields.kinds[y][x];
	if (kind != KIND_WATER) {
		return;
	}

	let belowKind = fields.kinds[y + 1][x];
	if (kind == KIND_WATER && belowKind == KIND_AIR) {
		fields.kinds[y][x] = KIND_AIR;
		fields.moved[y][x] = 1;
		triggerUpdateAround(x, y);
		addParticle(x, y, 0, 0);
		return;
	}

	let vx = fields.vx[y][x];
	if (vx == 0) {
		let ax, bx;
		if (Math.random() < 0.5) {
			ax = 1;
			bx = -1;
		} else {
			ax = -1;
			bx = 1;
		}

		let a = fields.kinds[y][x + ax];
		let b = fields.kinds[y][x + bx];

		if (a == KIND_AIR) {
			vx = fields.vx[y][x] = ax;
		} else if (b == KIND_AIR) {
			vx = fields.vx[y][x] = bx;
		} else {
			return;
		}
	}

	if (fields.kinds[y][x + vx] == KIND_AIR) {
		fields.kinds[y][x] = KIND_AIR;
		fields.kinds[y][x + vx] = kind;
		fields.vx[y][x + vx] = vx;
		fields.moved[y][x + vx] = 1;
		triggerUpdateAround(x, y);
		triggerUpdateAround(x + vx, y);

		if (fields.kinds[y + 1][x + vx] == KIND_AIR) {
			fields.kinds[y][x + vx] = KIND_AIR;
			addParticle(x, y, vx, 0);
		}

		return;
	}

	if (fields.kinds[y][x + vx] != KIND_AIR && fields.kinds[y][x - vx] == KIND_AIR) {
		fields.vx[y][x] = -vx;
		triggerUpdateAround(x, y);
		return;
	}

	if (Math.random() < 0.1) {
		fields.vx[y][x] = 0;
	}
}

function update() {
	handleMouse();

	let startTime = performance.now();

	let updates = pendingUpdates;
	pendingUpdates = [];
	pendingSet.clear();
	shuffle(updates);

	for (let [x, y] of updates) {
		if (x > 0 && x < WIDTH - 1 && y > 0 && y < HEIGHT - 1) {
			applyRules(x, y);
		}
	}

	for (let y = 0; y < HEIGHT; ++y) {
		for (let x = 0; x < WIDTH; ++x) {
			fields.moved[y][x] = 0;
		}
	}

	for (let i = 0; i < particles.length;) {
		const particle = particles[i];
		if (updateParticle(particle)) {
			i += 1;
		} else {
			particles[i] = particles[particles.length - 1];
			particles.pop();
		}
	}

	let simTimeMs = performance.now() - startTime;
	particleCountEl.innerText = particles.length;
	updateCountEl.innerText = updates.length;
	simTimeEl.innerText = simTimeMs;

	draw();
}

function stepSim() {
	update();
}

let updateIval = null;
function toggleSim() {
	if (updateIval) {
		clearInterval(updateIval);
		updateIval = null;
	} else {
		updateIval = setInterval(update, INTERVAL_MS);
	}
}
toggleSim();
