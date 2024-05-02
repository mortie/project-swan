let canvas = document.getElementById("canvas");
let ctx = canvas.getContext("2d");

const WIDTH = 50;
const HEIGHT = 50;

let matrix = [];
for (let y = 0; y < HEIGHT; ++y) {
	matrix.push([]);
	let row = matrix[y];
	for (let x = 0; x < WIDTH; ++x) {
		row.push(0);
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
			if (cell == 0) {
				continue;
			}

			if (cell == 255) {
				ctx.fillStyle = "rgb(255, 0, 0)";
			} else if (cell > 0 && cell < 255) {
				cell *= 20;
				let lightness = Math.max((255 - cell) / 255, 0);
				let blue = 200 * lightness + 55;
				let green = 90 * lightness + 10;
				ctx.fillStyle = `rgb(0, ${green}, ${blue})`;
			} else {
				ctx.fillStyle = "rgb(191, 40, 202)";
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
	let cellX = Math.floor(x / canvas.clientWidth * WIDTH)
	let cellY = Math.floor(y / canvas.clientHeight * HEIGHT)

	if (cellX < 1) cellX = 1;
	if (cellX > WIDTH - 2) cellX = WIDTH - 2;
	if (cellY < 1) cellY = 1;
	if (cellY > HEIGHT - 2) cellY = HEIGHT - 2;

	let cell = evt.buttons == 1 ? 255 : 1;
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
	if (x == 0 || x == WIDTH - 1 || y == 0 || y == HEIGHT - 1) {
		matrix[y][x] = 255;
		return;
	}

	// Water
	if (matrix[y][x] > 0 && matrix[y][x] < 255) {
		let self = matrix[y][x];
		let above = matrix[y - 1][x];
		let below = matrix[y + 1][x];
		let left = matrix[y][x - 1];
		let right = matrix[y][x + 1];

		if (below == 0) {
			matrix[y + 1][x] = 1;
			matrix[y][x] = 0;
		} else if (
			(isWater(above) && self < above + 1) ||
			(isWater(left) && self < left) ||
			(isWater(right) && self < right) ||
			(isWater(below) && self < below - 1)
		) {
			matrix[y][x] += 1;
		}

		self = matrix[y][x];

		if (self > 2 && left == 0) {
			matrix[y][x - 1] = self - 1;
			matrix[y][x] = 0;
		} else if (self > 2 && right == 0) {
			matrix[y][x + 1] = self - 1;
			matrix[y][x] = 0;
		} else if (self > 2 && above == 0) {
			matrix[y - 1][x] = self - 1;
			matrix[y][x] = 0;
		}
	}
}

function update() {
	for (let y = HEIGHT - 1; y >= 0; --y) {
		for (let x = 0; x < WIDTH; ++x) {
			updateCell(x, y);
		}
	}

	setTimeout(update, 100);
}
update();
