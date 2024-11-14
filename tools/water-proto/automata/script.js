let canvas = document.getElementById("canvas");
let ctx = canvas.getContext("2d");

const WIDTH = 100;
const HEIGHT = 100;
const INTERVAL_MS = 20;

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

function draw() {
	canvas.width = canvas.clientWidth * window.devicePixelRatio;
	canvas.height = canvas.clientHeight * window.devicePixelRatio;

	const pw = canvas.width / WIDTH;
	const ph = canvas.height / HEIGHT;

	for (let y = 0; y < HEIGHT; ++y) {
		for (let x = 0; x < WIDTH; ++x) {
			const kind = fields.kinds[y][x];
			const vx = fields.vx[y][x];
			if (kind == KIND_AIR) {
				continue;
			} else if (kind == KIND_SOLID) {
				ctx.fillStyle = "#000000";
			} else if (kind == KIND_WATER && vx == 0) {
				ctx.fillStyle = "#7777ff";
			} else if (kind == KIND_WATER && vx < 0) {
				ctx.fillStyle = "#6666ee";
			} else if (kind == KIND_WATER && vx > 0) {
				ctx.fillStyle = "#8888ff";
			}

			const px = x * pw;
			const py = y * ph;

			ctx.fillRect(px, py, pw, ph);

			if (fields.vx[y][x] < 0) {
				ctx.fillStyle = "rgba(255, 0, 0, 0.1)";
				ctx.fillRect(px, py, pw, ph);
			} else if (fields.vx[y][x] > 0) {
				ctx.fillStyle = "rgba(0, 0, 255, 0.1)";
				ctx.fillRect(px, py, pw, ph);
			}
		}
	}

	for (let [x, y] of pendingUpdates) {
		ctx.fillStyle = "rgba(10, 255, 10, 0.2)";
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

	if (cellX < 1) cellX = 1;
	if (cellX > WIDTH - 2) cellX = WIDTH - 2;
	if (cellY < 1) cellY = 1;
	if (cellY > HEIGHT - 2) cellY = HEIGHT - 2;

	let kind;
	if (evt.buttons == 1) {
		kind = KIND_SOLID;
	} else if (evt.shiftKey) {
		kind = KIND_AIR;
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

	triggerUpdateAround(cellX, cellY);
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
		fields.kinds[y + 1][x] = kind;
		fields.moved[y + 1][x] = 1;
		triggerUpdateAround(x, y);
		triggerUpdateAround(x, y + 1);
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
		fields.vx[y + 1][x] = vx;
		fields.moved[y][x + vx] = 1;
		triggerUpdateAround(x, y);
		triggerUpdateAround(x + vx, y);
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
