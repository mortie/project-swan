let canvas = document.getElementById("canvas");
let ctx = canvas.getContext("2d");

const WIDTH = 30;
const HEIGHT = 30;
const MAX = 255;
const ACCEL = 4;
const DAMPING = 0.93;

const TYPE_SOLID = 0;
const TYPE_AIR = 1;
const TYPE_WATER = 2;

let visualizeVelocities = false;

let fields = {
	type: [],
	mass: [],
	vx: [],
	vy: [],
	dvx: [],
	dvy: [],
};

function initMatrix(mat, w, h) {
	for (let y = 0; y < h; ++y) {
		let row = mat[y] = [];
		for (let x = 0; x < w; ++x) {
			row.push(0);
		}
	}
}

for (let f of ["type", "mass"]) {
	initMatrix(fields[f], WIDTH, HEIGHT);
}

for (let f of ["vx", "vy", "dvx", "dvy"]) {
	initMatrix(fields[f], WIDTH - 1, HEIGHT - 1);
}

for (let y = 1; y < HEIGHT - 1; ++y) {
	for (let x = 1; x < WIDTH - 1; ++x) {
		fields.type[y][x] = TYPE_AIR;
	}
}

function draw() {
	canvas.width = canvas.clientWidth * window.devicePixelRatio;
	canvas.height = canvas.clientHeight * window.devicePixelRatio;

	let cellWidth = canvas.width / WIDTH;
	let cellHeight = canvas.height / HEIGHT;

	if (visualizeVelocities) {
		for (let y = 1; y < HEIGHT; ++y) {
			for (let x = 1; x < WIDTH; ++x) {
				let vy = fields.vy[y - 1][x - 1];
				let avy = Math.abs(vy);
				if (avy > 0) {
					if (vy > 0) {
						ctx.fillStyle = "#aaff88";
					} else {
						ctx.fillStyle = "#ff6666";
					}
					ctx.fillRect(
						x * cellWidth + cellWidth / 2 - avy / 2,
						y * cellHeight - cellHeight / 2,
						avy, cellHeight);
				}
			}
		}

		for (let y = 1; y < HEIGHT; ++y) {
			for (let x = 1; x < WIDTH; ++x) {
				let vx = fields.vx[y - 1][x - 1];
				let avx = Math.abs(vx);
				if (avx > 0) {
					if (vx > 0) {
						ctx.fillStyle = "#aaff88";
					} else {
						ctx.fillStyle = "#ff6666";
					}
					ctx.fillRect(
						x * cellWidth - cellWidth / 2,
						y * cellHeight + cellHeight / 2 - avx / 2,
						cellWidth, avx);
				}
			}
		}
	}

	for (let y = 0; y < HEIGHT; ++y) {
		for (let x = 0; x < WIDTH; ++x) {

			let h = cellHeight;
			let t = fields.type[y][x];
			if (t == TYPE_AIR) {
				continue;
			}

			if (t == TYPE_SOLID) {
				ctx.fillStyle = "rgb(0, 0, 0)";
			} else {
				let m = fields.mass[y][x];
				ctx.fillStyle = "rgba(50, 50, 255, 0.6)";
				if (m < MAX * 0.8 || (y > 0 && fields.type[y - 1][x] != t)) {
					h *= (m / MAX);
				}
			}

			ctx.fillRect(x * cellWidth, y * cellHeight + cellHeight, cellWidth, -h);
		}
	}

	window.requestAnimationFrame(draw);
}
draw();

let currMouseEvt = null;
function onMouse(evt) {
	if (evt.buttons == 0) {
		currMouseEvt = null;
		return;
	} else {
		currMouseEvt = evt;
	}

	updateMouse();
}

function updateMouse() {
	let evt = currMouseEvt;
	if (!evt) {
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

	let type, mass;
	if (evt.buttons == 1 && evt.shiftKey) {
		type = TYPE_AIR;
		mass = 0;
	} else if (evt.buttons == 1) {
		type = TYPE_SOLID;
		mass = 0;
	} else {
		type = TYPE_WATER;
		mass = MAX;
	}

	function set(x, y) {
		fields.type[y][x] = type;
		fields.mass[y][x] = mass;
	}

	set(cellX, cellY);
}

canvas.addEventListener("mousedown", onMouse);
canvas.addEventListener("mousemove", onMouse);
canvas.addEventListener("contextmenu", evt => evt.preventDefault());
canvas.addEventListener("mouseup", () => currMouseEvt = null);

function updateVelocities(x, y) {
	let tSelf = fields.type[y][x];
	let mSelf = fields.mass[y][x];
	if (mSelf <= 0 && tSelf != TYPE_SOLID) {
		fields.type[y][x] = TYPE_AIR;
		fields.mass[y][x] = 0;
		tSelf = TYPE_AIR;
		return;
	}
	
	if (tSelf == TYPE_SOLID) {
		return;
	}

	// Types of surrounding cells
	let tLeft = fields.type[y][x - 1];
	let tRight = fields.type[y][x + 1];
	let tUp = fields.type[y - 1][x];
	let tDown = fields.type[y + 1][x];

	// Masses of surrounding cells
	let mLeft = fields.mass[y][x - 1];
	let mRight = fields.mass[y][x + 1];
	let mDown = fields.mass[y + 1][x];

	if (mDown < MAX && mSelf > 0 && (tDown == tSelf || tDown == TYPE_AIR)) {
		fields.vy[y][x - 1] += ACCEL;
		gravity = true;
	} else if (mSelf > MAX && (tUp == tSelf || tUp == TYPE_AIR)) {
		fields.vy[y - 1][x - 1] -= ACCEL;
	}

	if (mLeft < mSelf && (tLeft == tSelf || tLeft == TYPE_AIR)) {
		fields.vx[y - 1][x - 1] -= ACCEL;
	}

	if (mRight < mSelf && (tRight == tSelf || tRight == TYPE_AIR)) {
		fields.vx[y - 1][x] += ACCEL;
	}
}

function incompressiblize(x, y) {
	let tSelf = fields.type[y][x];
	if (tSelf == TYPE_SOLID || tSelf == TYPE_AIR) {
		return;
	}

	let mSelf = fields.mass[y][x];

	let div = 0;
	let sides = 0;
	let hasLeft = false;
	let hasRight = false;
	let hasUp = false;
	let hasDown = false;

	let tLeft = fields.type[y][x - 1];
	let tRight = fields.type[y][x + 1];
	let tUp = fields.type[y - 1][x];
	let tDown = fields.type[y + 1][x];

	if (tLeft == tSelf || tLeft == TYPE_AIR) {
		sides += 1;
		hasLeft = true;
		div += fields.vx[y - 1][x - 1];
	}

	if (tRight == tSelf || tRight == TYPE_AIR) {
		sides += 1;
		hasRight = true;
		div -= fields.vx[y - 1][x];
	}

	if (tUp == tSelf || tUp == TYPE_AIR) {
		sides += 1;
		hasUp = true;
		div += fields.vy[y - 1][x - 1];
	}

	if (tDown == tSelf || tDown == TYPE_AIR) {
		sides += 1;
		hasDown = true;
		div -= fields.vy[y][x - 1];
	}

	if (mSelf + div > MAX) {
		div = mSelf + div - MAX;
	} else if (mSelf + div < 0) {
		div = mSelf + div;
	} else {
		return;
	}

	div = Math.floor(div / sides);

	if (hasLeft) {
		fields.dvx[y - 1][x - 1] -= div;
	}

	if (hasRight) {
		fields.dvx[y - 1][x] += div;
	}

	if (hasUp) {
		fields.dvy[y - 1][x - 1] -= div;
	}

	if (hasDown) {
		fields.dvy[y][x - 1] += div;
	}
}

function moveMass(x, y) {
	let tSelf = fields.type[y][x];
	if (tSelf == TYPE_SOLID || tSelf == TYPE_AIR) {
		return;
	}

	let mSelf = fields.mass[y][x];

	// Velocities of surrounding cells
	let vLeft = fields.vx[y - 1][x - 1];
	let vRight = fields.vx[y - 1][x];
	let vUp = fields.vy[y - 1][x - 1];
	let vDown = fields.vy[y][x - 1];

	// Types of surrounding cells
	let tUp = fields.type[y - 1][x];
	let tDown = fields.type[y + 1][x];

	if (vUp < 0 && (tUp == tSelf || tUp == TYPE_AIR)) {
		let diff = Math.min(-vUp, mSelf);
		fields.mass[y - 1][x] += diff;
		fields.mass[y][x] -= diff;
		fields.type[y - 1][x] = tSelf;
		mSelf -= diff;
	}

	if (vDown > 0 && (tDown == tSelf || tDown == TYPE_AIR)) {
		let diff = Math.min(vDown, mSelf);
		fields.mass[y + 1][x] += diff;
		fields.mass[y][x] -= diff;
		fields.type[y + 1][x] = tSelf;
		mSelf -= diff;
	}

	let vA, vB;
	let ax, bx;
	if (Math.random() > 0.5) {
		ax = -1;
		bx = 1;
		vA = -vLeft;
		vB = vRight;
	} else {
		ax = 1;
		bx = -1;
		vA = vRight;
		vB = -vLeft;
	}

	let tA = fields.type[y][x + ax];
	let tB = fields.type[y][x + bx];

	if (vA > 0 && (tA == tSelf || tA == TYPE_AIR)) {
		let diff = Math.min(vA, mSelf);
		fields.mass[y][x + ax] += diff;
		fields.mass[y][x] -= diff;
		fields.type[y][x + ax] = tSelf;
		mSelf -= diff;
	}

	if (vB > 0 && (tB == tSelf || tB == TYPE_AIR)) {
		let diff = Math.min(vB, mSelf);
		fields.mass[y][x + bx] += diff;
		fields.mass[y][x] -= diff;
		fields.type[y][x + bx] = tSelf;
		mSelf -= diff;
	}
}

function update() {
	for (let y = 1; y < HEIGHT - 1; ++y) {
		for (let x = 1; x < WIDTH - 1; ++x) {
			updateVelocities(x, y);
		}
	}

	for (let y = 0; y < HEIGHT - 1; ++y) {
		for (let x = 0; x < WIDTH - 1; ++x) {
			let vx = fields.vx[y][x];
			let vy = fields.vy[y][x];
			let sx = Math.sign(vx);
			let sy = Math.sign(vy);

			fields.vx[y][x] = Math.floor(Math.abs(vx) * DAMPING) * sx;
			fields.vy[y][x] = Math.floor(Math.abs(vy) * DAMPING) * sy;
		}
	}

	for (let y = 1; y < HEIGHT - 1; ++y) {
		for (let x = 1; x < WIDTH - 1; ++x) {
			incompressiblize(x, y);
		}
	}

	for (let y = 0; y < HEIGHT - 1; ++y) {
		for (let x = 0; x < WIDTH - 1; ++x) {
			fields.vx[y][x] += fields.dvx[y][x];
			fields.dvx[y][x] = 0;
			fields.vy[y][x] += fields.dvy[y][x];
			fields.dvy[y][x] = 0;
		}
	}

	for (let y = 1; y < HEIGHT - 1; ++y) {
		for (let x = 1; x < WIDTH - 1; ++x) {
			moveMass(x, y);
		}
	}

	updateMouse();
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
	console.log(fields);
}
