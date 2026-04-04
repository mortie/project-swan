#pragma once

#include "../Prefab.h"
#include "tiles.h"
#include <array>

namespace CoreMod::PineCrown {

inline Prefab::Map map = {
	{'#', tiles::pine__leaves},
	{'L', tiles::pine__leaves__left},
	{'R', tiles::pine__leaves__right},
	{'T', tiles::pine__top},
	{'X', tiles::pine__cross},
	{'|', tiles::pine__center},
	{'(', tiles::pine__branch__left},
	{')', tiles::pine__branch__right},
	{'<', tiles::pine__stub__left},
	{'>', tiles::pine__stub__right},
};

inline Prefab variant1(map, {
	"LTR",
	"<X>",
});

inline Prefab variant2(map, {
	"LTR",
	"<X>",
	"#|#",
	"<X>",
	"<X>",
});

inline Prefab variant3(map, {
	" LTR ",
	"L<X>R",
	"<(X)>",
	"<(X)>",
});

inline Prefab variant4(map, {
	"LTR",
	"<X>",
	"<X>",
	"<X>",
	"<X>",
	"<X>",
});

inline Prefab variant5(map, {
	" L#T#R ",
	" <(X)> ",
	"L##|##R",
	"<((X))>",
});

inline std::array variants = {
	&variant1,
	&variant2,
	&variant3,
	&variant4,
	&variant5,
};

}
