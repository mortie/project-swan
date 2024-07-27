let canvas = document.getElementById("canvas");
let ctx = canvas.getContext("2d");
let maxPEl = document.getElementById("maxP");

const WIDTH = 30;
const HEIGHT = 30;
const INTERVAL_MS = 50;
const GRAVITY = 10; // m/s/s
const DENSITY = 1;

const GRID_WIDTH = WIDTH + 2;
const GRID_HEIGHT = HEIGHT + 2;

let fields = {
	vx: [],
	vy: [],
	dvx: [],
	dvy: [],
	solid: [],
	pressure: [],
};

function initMatrix(mat, w, h) {
	for (let y = 0; y < h; ++y) {
		let row = mat[y] = [];
		for (let x = 0; x < w; ++x) {
			row.push(0);
		}
	}
}

for (let f of ["vx", "vy", "dvx", "dvy"]) {
	initMatrix(fields[f], WIDTH + 1, HEIGHT + 1);
}

for (let f of ["solid", "pressure"]) {
	initMatrix(fields[f], WIDTH + 2, HEIGHT + 2);
}

for (let y = 0; y < HEIGHT + 2; ++y) {
	fields.solid[y][0] = fields.solid[y][WIDTH + 1] = 1;
}
for (let x = 0; x < WIDTH + 2; ++x) {
	fields.solid[0][x] = fields.solid[HEIGHT + 1][x] = 1;
}

function draw() {
	canvas.width = canvas.clientWidth * window.devicePixelRatio;
	canvas.height = canvas.clientHeight * window.devicePixelRatio;

	let cellWidth = canvas.width / GRID_WIDTH;
	let cellHeight = canvas.height / GRID_HEIGHT;

	let maxP = -Infinity;
	let minP = Infinity;
	for (let y = 0; y < GRID_HEIGHT; ++y) {
		for (let x = 0; x < GRID_WIDTH; ++x) {
			let p = fields.pressure[y][x];
			if (p > maxP) {
				maxP = p;
			}
			if (p < minP) {
				minP = p;
			}
		}
	}
	maxPEl.innerText = "+" + Math.round(maxP) + " / -" + Math.abs(Math.round(minP));

	for (let y = 0; y < GRID_HEIGHT; ++y) {
		for (let x = 0; x < GRID_WIDTH; ++x) {
			if (fields.solid[y][x]) {
				ctx.fillStyle = "#0000cc";
			} else if (fields.pressure[y][x] > 0.1) {
				let frac = 1 - (fields.pressure[y][x] / maxP);
				ctx.fillStyle = "rgb(" + (frac * 255) + ", 0, 0)";
			} else if (fields.pressure[y][x] < -0.1) {
				let frac = 1 - (fields.pressure[y][x] / minP);
				ctx.fillStyle = "rgb(0, " + (frac * 255) + ", 0)";
			} else {
				ctx.fillStyle = "#ffffff";
			}

			ctx.fillRect(x * cellWidth, y * cellHeight, cellWidth, cellHeight);
		}
	}

	window.requestAnimationFrame(draw);
}
draw();

function onMouse(evt) {
	if (evt.buttons == 0) {
		return;
	}

	let x = evt.clientX - canvas.offsetLeft;
	let y = evt.clientY - canvas.offsetTop;
	let cellX = Math.floor(x / canvas.clientWidth * GRID_WIDTH)
	let cellY = Math.floor(y / canvas.clientHeight * GRID_HEIGHT)

	if (cellX < 1) cellX = 1;
	if (cellX > GRID_WIDTH - 2) cellX = GRID_WIDTH - 2;
	if (cellY < 1) cellY = 1;
	if (cellY > GRID_HEIGHT - 2) cellY = GRID_HEIGHT - 2;

	if (evt.buttons == 1) {
		fields.solid[cellY][cellX] = 1;
	} else {
		fields.solid[cellY][cellX] = 0;
		fields.vx[cellY][cellX] = 1;
		fields.vx[cellY][cellX + 1] = 1;
	}
}

canvas.addEventListener("mousedown", onMouse);
canvas.addEventListener("mousemove", onMouse);
canvas.addEventListener("contextmenu", evt => evt.preventDefault());

function updateCell(x, y, dt) {
	let div = 0;

	let sides = 0;
	let hasLeft = false;
	let hasRight = false;
	let hasAbove = false
	let hasBelow = false;

	div += fields.vx[y][x];
	div -= fields.vx[y][x + 1];
	div += fields.vy[y][x];
	div -= fields.vy[y + 1][x];

	if (!fields.solid[y + 1][x]) {
		sides += 1;
		hasLeft = true;
	}
	if (!fields.solid[y + 1][x + 2]) {
		sides += 1;
		hasRight = true;
	}
	if (!fields.solid[y][x + 1]) {
		sides += 1;
		hasAbove = true;
	}
	if (!fields.solid[y + 2][x + 1]) {
		sides += 1;
		hasBelow = true;
	}

	if (sides != 0) {
		div /= sides;
	}
	div *= -1.0;

	if (hasLeft) {
		fields.dvx[y][x] += div;
	}
	if (hasRight) {
		fields.dvx[y][x + 1] -= div;
	}
	if (hasAbove) {
		fields.dvy[y][x] += div;
	}
	if (hasBelow) {
		fields.dvy[y + 1][x] -= div;
	}

	fields.pressure[y + 1][x + 1] += (div * DENSITY) / dt;
}

function updateCells(dt) {
	for (let y = 0; y < HEIGHT; ++y) {
		for (let x = 0; x < WIDTH; ++x) {
			updateCell(x, y, dt);
		}
	}
}

function addForces(dt) {
	let g = GRAVITY * dt;
	for (let y = 0; y < HEIGHT + 1; ++y) {
		for (let x = 0; x < WIDTH + 1; ++x) {
			if (fields.solid[y][x + 1]) {
				continue;
			}

			if (fields.solid[y + 1][x + 1]) {
				continue;
			}

			fields.vy[y][x] += g;
		}
	}
}

function update() {
	let dt = INTERVAL_MS / 1000;

	for (let y = 0; y < HEIGHT + 1; ++y) {
		for (let x = 0; x < WIDTH + 1; ++x) {
			fields.pressure[y][x] = 0;
		}
	}

	addForces(dt);
	updateCells(dt);

	for (let y = 0; y < HEIGHT + 1; ++y) {
		for (let x = 0; x < WIDTH + 1; ++x) {
			fields.vx[y][x] += fields.dvx[y][x];
			fields.vy[y][x] += fields.dvy[y][x];
			fields.dvx[y][x] = 0;
			fields.dvy[y][x] = 0;
		}
	}
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

function stepSim() {
	update();
	console.log(fields);
}


