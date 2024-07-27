let canvas = document.getElementById("canvas");
let ctx = canvas.getContext("2d");

const WIDTH = 30;
const HEIGHT = 30;

const SOLID = 255;
const WATER_MAX = 1;

let matrix = [];
let adj = [];
for (let y = 0; y < HEIGHT; ++y) {
	matrix.push([]);
	adj.push([]);
	for (let x = 0; x < WIDTH; ++x) {
		matrix[y].push(0);
		adj[y].push(0);
	}
}

function draw() {
	canvas.width = canvas.clientWidth * window.devicePixelRatio;
	canvas.height = canvas.clientHeight * window.devicePixelRatio;

	let cellWidth = canvas.width / WIDTH;
	let cellHeight = canvas.height / HEIGHT;

	for (let y = 0; y < HEIGHT; ++y) {
		for (let x = 0; x < WIDTH; ++x) {
			let cell = matrix[y][x];
			if (cell <= 0) {
				continue;
			}

			/*
			if (cell != SOLID && matrix[y - 1][x] > 0 && matrix[y - 1][x] != SOLID) {
				ctx.fillStyle = "rgb(70, 70, 255)";
				ctx.fillRect(x * cellWidth, y * cellHeight, cellWidth, cellHeight);
			}
			*/

			let h = cellHeight;
			if (cell == SOLID) {
				ctx.fillStyle = "rgb(255, 0, 0)";
			} else if (cell > 0 && cell <= WATER_MAX) {
				ctx.fillStyle = "rgb(50, 50, 255)";
				h *= (cell / WATER_MAX);
			} else {
				ctx.fillStyle = "rgb(40, 40, 230)";
			}

			ctx.fillRect(x * cellWidth, y * cellHeight + cellHeight, cellWidth, -h);
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
	let cellX = Math.floor(x / canvas.clientWidth * WIDTH)
	let cellY = Math.floor(y / canvas.clientHeight * HEIGHT)

	if (cellX < 1) cellX = 1;
	if (cellX > WIDTH - 2) cellX = WIDTH - 2;
	if (cellY < 1) cellY = 1;
	if (cellY > HEIGHT - 2) cellY = HEIGHT - 2;

	let cell = evt.buttons == 1 ? 255 : WATER_MAX;
	matrix[cellY - 1][cellX - 1] = cell;
	matrix[cellY - 1][cellX] = cell;
	matrix[cellY - 1][cellX + 1] = cell;
	matrix[cellY][cellX - 1] = cell;
	matrix[cellY][cellX] = cell;
	matrix[cellY][cellX + 1] = cell;
	matrix[cellY + 1][cellX - 1] = cell;
	matrix[cellY + 1][cellX] = cell;
	matrix[cellY + 1][cellX + 1] = cell;
}

canvas.addEventListener("mousedown", onMouse);
canvas.addEventListener("mousemove", onMouse);
canvas.addEventListener("contextmenu", evt => evt.preventDefault());

function isWater(cell) {
	return cell > 0 && cell < 255;
}

function updateCell(x, y) {
	let self = matrix[y][x];
	if (self == SOLID) {
		return;
	}

	let above = matrix[y - 1][x];

	let ax, bx;
	if (Math.random() > 0.5) {
		ax = x - 1;
		bx = x + 1;
	} else {
		ax = x + 1;
		bx = x - 1;
	}
	let a = matrix[y][ax];
	let b = matrix[y][bx];

	if (self > WATER_MAX && above != SOLID && Math.random() > 0.75) {
		let diff = self - WATER_MAX;
		adj[y - 1][x] += diff;
		adj[y][x] -= diff;
		self -= diff;
	}

	if (a != SOLID && a < WATER_MAX && self > a) {
		let diff = (self - a) / 5;
		adj[y][ax] += diff;
		adj[y][x] -= diff;
		self -= diff;
	}

	if (b != SOLID && b < WATER_MAX && self > b) {
		let diff = (self - b) / 5;
		adj[y][bx] += diff;
		adj[y][x] -= diff;
		self -= diff;
	}
}

function updateCell2(x, y) {
	let self = matrix[y][x];
	if (self == SOLID) {
		return;
	}

	let below = matrix[y + 1][x];

	if (self > 0 && below < WATER_MAX) {
		let diff = Math.min(WATER_MAX - below, self);
		adj[y + 1][x] += diff;
		adj[y][x] -= diff;
		self -= diff;
	}
}

function update() {
	for (let y = 1; y < HEIGHT - 1; ++y) {
		for (let x = 1; x < WIDTH - 1; ++x) {
			updateCell(x, y);
		}
	}

	for (let y = 1; y < HEIGHT - 1; ++y) {
		for (let x = 1; x < WIDTH - 1; ++x) {
			matrix[y][x] += adj[y][x];
			adj[y][x] = 0;
		}
	}

	for (let y = 1; y < HEIGHT - 1; ++y) {
		for (let x = 1; x < WIDTH - 1; ++x) {
			updateCell2(x, y);
		}
	}

	for (let y = 1; y < HEIGHT - 1; ++y) {
		for (let x = 1; x < WIDTH - 1; ++x) {
			matrix[y][x] += adj[y][x];
			adj[y][x] = 0;
		}
	}

	for (let y = 0; y < HEIGHT; ++y) {
		matrix[y][0] = SOLID;
		matrix[y][WIDTH - 1] = SOLID;
	}

	for (let x = 0; x < WIDTH; ++x) {
		matrix[0][x] = SOLID;
		matrix[HEIGHT - 1][x] = SOLID;
	}
}

let updateIval = null;
function toggleSim() {
	if (updateIval) {
		clearInterval(updateIval);
		updateIval = null;
	} else {
		updateIval = setInterval(update, 50);
	}
}
toggleSim();

function stepSim() {
	update();
}
