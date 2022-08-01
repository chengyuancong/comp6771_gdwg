#include "gdwg/graph.hpp"
#include <catch2/catch.hpp>

TEST_CASE("Extractor <<") {
	SECTION("Empty") {
		auto g = gdwg::graph<std::string, int>{};
		auto out = std::ostringstream{};
		out << g;
		auto const expect = std::string("");
		CHECK(out.str() == expect);
	}

	SECTION("Easy") {
		auto g = gdwg::graph<std::string, int>{"how", "are", "you"};
		g.insert_edge("how", "are", 1);
		g.insert_edge("are", "you", 2);
		g.insert_edge("are", "are", 3);
		auto out = std::ostringstream{};
		out << g;
		auto const expect = std::string_view(R"(are (
  are | 3
  you | 2
)
how (
  are | 1
)
you (
)
)");
		CHECK(out.str() == expect);
	}

	SECTION("Hard") {
		auto g = gdwg::graph<int, int>{1, 2, 3, 4, 5, 6};
		g.insert_edge(4, 1, -4);
		g.insert_edge(3, 2, 2);
		g.insert_edge(2, 4, 2);
		g.insert_edge(2, 1, 1);
		g.insert_edge(6, 2, 5);
		g.insert_edge(6, 3, 10);
		g.insert_edge(1, 5, -1);
		g.insert_edge(3, 6, -8);
		g.insert_edge(4, 5, 3);
		g.insert_edge(5, 2, 7);

		g.insert_node(64);
		auto out = std::ostringstream{};
		out << g;
		auto const expect = std::string_view(R"(1 (
  5 | -1
)
2 (
  1 | 1
  4 | 2
)
3 (
  2 | 2
  6 | -8
)
4 (
  1 | -4
  5 | 3
)
5 (
  2 | 7
)
6 (
  2 | 5
  3 | 10
)
64 (
)
)");
		CHECK(out.str() == expect);
	}
}

TEST_CASE("Comparison") {
	SECTION("Empty graphs are equal") {
		auto g1 = gdwg::graph<std::string, int>{};
		auto g2 = gdwg::graph<std::string, int>{};
		CHECK(g1 == g2);
	}

	SECTION("Graphs have no edges") {
		auto g1 = gdwg::graph<std::string, int>{"how", "are", "you"};
		auto g2 = gdwg::graph<std::string, int>{"how", "are", "you", "hello"};
		auto g3 = gdwg::graph<std::string, int>{"how", "are", "you", "hello"};
		auto g4 = gdwg::graph<std::string, int>{"how", "you", "are"};
		CHECK_FALSE(g1 == g2);
		CHECK(g2 == g3);
		CHECK(g1 == g4);
	}

	SECTION("Graphs have edges") {
		auto g1 = gdwg::graph<std::string, int>{"how", "are", "you"};
		g1.insert_edge("how", "you", 1);
		g1.insert_edge("how", "how", 2);
		g1.insert_edge("are", "you", 3);

		auto g2 = gdwg::graph<std::string, int>{"are", "you", "how"};
		g2.insert_edge("how", "you", 1);
		g2.insert_edge("how", "how", 2);
		g2.insert_edge("are", "you", 3);

		CHECK(g1 == g1);
		CHECK(g1 == g2);

		g2.insert_node("?");
		CHECK_FALSE(g1 == g2);

		g1.insert_node("?");
		CHECK(g1 == g2);

		g1.insert_edge("how", "?", 4);
		CHECK_FALSE(g1 == g2);

		g2.insert_edge("how", "?", 4);
		CHECK(g1 == g2);

		g1.insert_edge("how", "?", 5);
		g2.insert_edge("how", "?", 6);
		CHECK_FALSE(g1 == g2);
	}
}
