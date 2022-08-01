#include "gdwg/graph.hpp"
#include <catch2/catch.hpp>

TEST_CASE("Iterator test") {
	auto g = gdwg::graph<std::string, int>{"how", "are", "you"};
	g.insert_edge("how", "are", 1);
	g.insert_edge("how", "you", 2);
	g.insert_edge("are", "you", 3);
	auto const const_g = g;

	SECTION("Begin and end") {
		auto g_empty = gdwg::graph<std::string, int>{};
		CHECK(g_empty.begin() == g_empty.end());
		CHECK_FALSE(g.begin() == g.end());
	}

	SECTION("Dereference *") {
		auto it = g.begin();
		CHECK((*it).from == "are");
		CHECK((*it).to == "you");
		CHECK((*it).weight == 3);
	}

	SECTION("Traversal ++itor") {
		auto it = g.begin();
		auto it2 = ++it;
		CHECK((*it2).from == "how");
		CHECK((*it2).to == "are");
		CHECK((*it2).weight == 1);
		CHECK((*it).from == "how");
		CHECK((*it).to == "are");
		CHECK((*it).weight == 1);
	}
	SECTION("Traversal itor++") {
		auto it = g.begin();
		auto it2 = it++;
		CHECK((*it2).from == "are");
		CHECK((*it2).to == "you");
		CHECK((*it2).weight == 3);
		CHECK((*it).from == "how");
		CHECK((*it).to == "are");
		CHECK((*it).weight == 1);
	}
	SECTION("Traversal --itor") {
		auto it = g.end();
		auto it2 = --it;
		CHECK((*it).from == "how");
		CHECK((*it).to == "you");
		CHECK((*it).weight == 2);
		CHECK((*it2).from == "how");
		CHECK((*it2).to == "you");
		CHECK((*it2).weight == 2);
		auto it3 = --it2;
		CHECK((*it3).from == "how");
		CHECK((*it3).to == "are");
		CHECK((*it3).weight == 1);
		CHECK((*it2).from == "how");
		CHECK((*it2).to == "are");
		CHECK((*it2).weight == 1);
	}
	SECTION("Traversal itor--") {
		auto it = g.end();
		auto it2 = it--;
		CHECK(it2 == g.end());
		CHECK((*it).from == "how");
		CHECK((*it).to == "you");
		CHECK((*it).weight == 2);
		auto it3 = it--;
		CHECK((*it3).from == "how");
		CHECK((*it3).to == "you");
		CHECK((*it3).weight == 2);
		CHECK((*it).from == "how");
		CHECK((*it).to == "are");
		CHECK((*it).weight == 1);
	}

	SECTION("Iterator comparison") {
		auto it = g.begin();
		// check equality
		CHECK(it == g.find("are", "you", 3));
		++it;
		CHECK(it == g.find("how", "are", 1));
		++it;
		CHECK(it == g.find("how", "you", 2));
		++it;
		CHECK(it == g.end());

		// check inequality
		auto it2 = g.begin();
		CHECK_FALSE(it2 == g.end());
		CHECK_FALSE(++it2 == g.end());
		CHECK_FALSE(++it2 == g.end());
		CHECK_FALSE(--it2 == g.end());
		CHECK_FALSE(--it2 == g.end());
		CHECK_FALSE(g.find("are", "you", 3) == g.end());
		CHECK_FALSE(g.find("how", "are", 1) == g.find("how", "you", 2));
		CHECK_FALSE(g.find("how", "are", 1) == g.begin());
	}
}
