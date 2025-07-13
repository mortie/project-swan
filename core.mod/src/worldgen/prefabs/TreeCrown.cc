#include "TreeCrown.h"

namespace CoreMod::TreeCrown {

Prefab::Map map = {
	{'#', "core::tree::leaves"},
	{'L', "core::tree::leaves::left"},
	{'R', "core::tree::leaves::right"},
	{'T', "core::tree::top"},
	{'X', "core::tree::cross"},
	{'|', "core::tree::center"},
	{'(', "core::tree::branch::left"},
	{')', "core::tree::branch::right"},
	{'<', "core::tree::stub::left"},
	{'>', "core::tree::stub::right"},
};

Prefab::Data variant1 = {
	"LTR",
	"<X>",
};

Prefab::Data variant2 = {
	" LTR ",
	"L<X>R",
	"<(X)>",
};

Prefab::Data variant3 = {
	"  LTR  ",
	" L<X>R ",
	"L<(X)>R",
	"<((X))>",
};

Prefab::Data variant4 = {
	"LTR",
	"#|#",
};

Prefab::Data variant5 = {
	" L#T#R ",
	" <(X)> ",
	"L##|##R",
	"<((X))>",
};

}
