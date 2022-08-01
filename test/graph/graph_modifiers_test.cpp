#include "gdwg/graph.hpp"
#include <catch2/catch.hpp>
#include <sstream>
#include <string>
#include <string_view>

TEST_CASE("Insert node") {
	SECTION("Stored in heap") {
		auto g = gdwg::graph<std::string, int>();
		// Test if g has its own copied resource that it has stored
		{
			auto str = std::string("hello");
			g.insert_node(str);
		}
		CHECK(g.is_node("hello"));
	}

	SECTION("Insert duplicate node") {
		auto g = gdwg::graph<std::string, int>();
		g.insert_node("hello");
		CHECK(g.is_node("hello"));
		CHECK_FALSE(g.insert_node("hello"));
	}
}

TEST_CASE("Insert edge") {
	SECTION("Stored in heap") {
		auto g = gdwg::graph<std::string, std::string>();
		g.insert_node("hello");
		g.insert_node("world");
		{
			auto str = std::string("wonderful");
			g.insert_edge("hello", "world", str);
		}
		CHECK(g.find("hello", "world", "wonderful") != g.end());
	}

	SECTION("Insert reflexive edge") {
		auto g = gdwg::graph<std::string, int>();
		g.insert_node("hello");
		g.insert_edge("hello", "hello", 1);
		CHECK(g.find("hello", "hello", 1) != g.end());
	}

	SECTION("Insert different weight") {
		auto g = gdwg::graph<std::string, int>();
		g.insert_node("hello");
		g.insert_node("world");
		// normal edge
		CHECK(g.insert_edge("hello", "world", 1));
		CHECK(g.insert_edge("hello", "world", 2));
		CHECK(g.find("hello", "world", 1) != g.end());
		CHECK(g.find("hello", "world", 2) != g.end());
		// reflexive edge
		CHECK(g.insert_edge("hello", "hello", 1));
		CHECK(g.insert_edge("hello", "hello", 2));
		CHECK(g.find("hello", "hello", 1) != g.end());
		CHECK(g.find("hello", "hello", 2) != g.end());
	}

	SECTION("Insert duplicate edge") {
		auto g = gdwg::graph<std::string, int>();
		g.insert_node("hello");
		g.insert_node("world");
		// normal edge
		g.insert_edge("hello", "world", 1);
		CHECK(g.find("hello", "world", 1) != g.end());
		CHECK_FALSE(g.insert_edge("hello", "world", 1));
		// reflexive edge
		g.insert_edge("hello", "hello", 1);
		CHECK(g.find("hello", "hello", 1) != g.end());
		CHECK_FALSE(g.insert_edge("hello", "hello", 1));
	}

	SECTION("Exception: either src or dst node does not exist") {
		auto g = gdwg::graph<std::string, int>();
		g.insert_node("how");
		g.insert_node("are");
		g.insert_node("you");
		// src does not exist
		CHECK_THROWS_MATCHES(g.insert_edge("hello", "how", 1),
		                     std::runtime_error,
		                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::insert_edge "
		                                              "when either src or dst node does not exist"));

		// dst does not exist
		CHECK_THROWS_MATCHES(g.insert_edge("how", "hello", 1),
		                     std::runtime_error,
		                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::insert_edge "
		                                              "when either src or dst node does not exist"));

		// both src and dst does not exist
		CHECK_THROWS_MATCHES(g.insert_edge("hello", "world", 1),
		                     std::runtime_error,
		                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::insert_edge "
		                                              "when either src or dst node does not exist"));
	}
}

TEST_CASE("Replace node") {
	auto g = gdwg::graph<std::string, int>();
	SECTION("New data does not exist") {
		g.insert_node("hello");
		g.insert_node("world");
		g.insert_edge("hello", "world", 1);
		g.insert_edge("world", "hello", 2);
		g.insert_edge("hello", "hello", 3);
		// replace
		CHECK(g.replace_node("hello", "wonderful"));
		// check old nodes and edges not exist
		CHECK_FALSE(g.is_node("hello"));
		CHECK(g.find("hello", "world", 1) == g.end());
		CHECK(g.find("world", "hello", 2) == g.end());
		CHECK(g.find("hello", "hello", 3) == g.end());
		// check new nodes and edges exist
		CHECK(g.is_node("wonderful"));
		CHECK(g.find("wonderful", "world", 1) != g.end());
		CHECK(g.find("world", "wonderful", 2) != g.end());
		CHECK(g.find("wonderful", "wonderful", 3) != g.end());
	}

	SECTION("New data already exists") {
		g.insert_node("how");
		g.insert_node("are");
		g.insert_node("you");
		g.insert_edge("how", "are", 1);
		g.insert_edge("are", "you", 2);
		g.insert_edge("you", "how", 3);
		g.insert_edge("how", "how", 4);
		// replace
		CHECK_FALSE(g.replace_node("how", "are"));
		// check old nodes and edges exist
		CHECK(g.is_node("how"));
		CHECK(g.is_node("are"));
		CHECK(g.is_node("you"));
		CHECK(g.find("how", "are", 1) != g.end());
		CHECK(g.find("are", "you", 2) != g.end());
		CHECK(g.find("you", "how", 3) != g.end());
		CHECK(g.find("how", "how", 4) != g.end());
		// check new edges not exist
		CHECK(g.find("are", "are", 1) == g.end());
		CHECK(g.find("you", "are", 3) == g.end());
		CHECK(g.find("how", "are", 4) == g.end());
		// replace with same value
		CHECK_FALSE(g.replace_node("how", "how"));
	}

	SECTION("Exception: old data does not exist") {
		CHECK_THROWS_MATCHES(g.replace_node("hi", "world"),
		                     std::runtime_error,
		                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::replace_node "
		                                              "on a node that doesn't exist"));
	}
}

TEST_CASE("Merge replace node") {
	auto g = gdwg::graph<std::string, int>();
	g.insert_node("how");
	g.insert_node("are");
	g.insert_node("you");
	g.insert_edge("how", "are", 1);
	g.insert_edge("are", "you", 2);
	g.insert_edge("are", "are", 3);

	SECTION("Merge raplace by same value") {
		// replace
		g.merge_replace_node("how", "how");
		// check if graph is same
		auto oss = std::ostringstream{};
		oss << g;
		auto const expected = std::string_view(R"(are (
  are | 3
  you | 2
)
how (
  are | 1
)
you (
)
)");
		CHECK(oss.str() == expected);
	}

	SECTION("No duplicate edges after merge replace") {
		// replace
		g.merge_replace_node("are", "how");
		auto oss = std::ostringstream{};
		oss << g;
		auto const expected = std::string_view(R"(how (
  how | 1
  how | 3
  you | 2
)
you (
)
)");
		CHECK(oss.str() == expected);
	}

	SECTION("Has duplicate edges after merge replace") {
		g.insert_edge("how", "you", 2);
		// replace
		g.merge_replace_node("are", "how");
		auto oss = std::ostringstream{};
		oss << g;
		auto const expected = std::string_view(R"(how (
  how | 1
  how | 3
  you | 2
)
you (
)
)");
		CHECK(oss.str() == expected);
	}

	SECTION("Exception: either src or dst node does not exist") {
		// src does not exist
		CHECK_THROWS_MATCHES(g.merge_replace_node("hello", "how"),
		                     std::runtime_error,
		                     Catch::Matchers::Message("Cannot call gdwg::graph<N, "
		                                              "E>::merge_replace_node on old or new data if "
		                                              "they don't exist in the graph"));

		// dst does not exist
		CHECK_THROWS_MATCHES(g.merge_replace_node("how", "hello"),
		                     std::runtime_error,
		                     Catch::Matchers::Message("Cannot call gdwg::graph<N, "
		                                              "E>::merge_replace_node on old or new data if "
		                                              "they don't exist in the graph"));

		// both src and dst does not exist
		CHECK_THROWS_MATCHES(g.merge_replace_node("hello", "world"),
		                     std::runtime_error,
		                     Catch::Matchers::Message("Cannot call gdwg::graph<N, "
		                                              "E>::merge_replace_node on old or new data if "
		                                              "they don't exist in the graph"));
	}
}

TEST_CASE("Erase node") {
	auto g = gdwg::graph<std::string, int>{"how", "are", "you"};
	SECTION("Node exists") {
		CHECK(g.is_node("how"));
		CHECK(g.erase_node("how"));
		CHECK_FALSE(g.is_node("how"));
	}

	SECTION("Node does not exist") {
		CHECK_FALSE(g.erase_node("hello"));
	}

	SECTION("Remove relevant edges") {
		g.insert_edge("how", "are", 1);
		g.insert_edge("are", "you", 2);
		g.insert_edge("you", "how", 3);
		g.insert_edge("how", "how", 4);
		CHECK(g.erase_node("how"));
		CHECK_FALSE(g.is_node("how"));
		CHECK(g.find("how", "are", 1) == g.end());
		CHECK(g.find("are", "you", 2) != g.end());
		CHECK(g.find("you", "how", 3) == g.end());
		CHECK(g.find("how", "how", 4) == g.end());
	}
}

TEST_CASE("Erase edge (src, dst, weight)") {
	auto g = gdwg::graph<std::string, int>{"how", "are", "you"};
	g.insert_edge("how", "are", 1);
	g.insert_edge("are", "you", 2);
	g.insert_edge("you", "how", 3);
	g.insert_edge("how", "how", 4);
	SECTION("Edge exists") {
		g.insert_edge("how", "are", 2);
		CHECK(g.erase_edge("how", "are", 1));
		CHECK(g.find("how", "are", 1) == g.end());
		CHECK(g.find("how", "are", 2) != g.end());
	}
	SECTION("Edge does not exist") {
		CHECK_FALSE(g.erase_edge("how", "how", 1));
	}
	SECTION("Exception: either src or dst node does not exist") {
		// src does not exist
		CHECK_THROWS_MATCHES(g.erase_edge("hello", "how", 1),
		                     std::runtime_error,
		                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::erase_edge on "
		                                              "src or dst if they don't exist in the graph"));

		// dst does not exist
		CHECK_THROWS_MATCHES(g.erase_edge("how", "hello", 1),
		                     std::runtime_error,
		                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::erase_edge on "
		                                              "src or dst if they don't exist in the graph"));

		// both src and dst does not exist
		CHECK_THROWS_MATCHES(g.erase_edge("hello", "world", 1),
		                     std::runtime_error,
		                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::erase_edge on "
		                                              "src or dst if they don't exist in the graph"));
	}
}

TEST_CASE("Erase edge (iterator)") {
	auto g = gdwg::graph<std::string, int>{"how", "are", "you"};

	SECTION("Erase one edge") {
		g.insert_edge("are", "you", 2);
		CHECK(g.find("are", "you", 2) != g.end());
		CHECK(g.erase_edge(g.begin()) == g.end());
		CHECK(g.find("are", "you", 2) == g.end());
	}

	SECTION("Erase multiple edges") {
		g.insert_edge("how", "are", 1);
		g.insert_edge("are", "you", 2);
		g.insert_edge("you", "how", 3);
		g.insert_edge("how", "how", 4);
		CHECK(g.erase_edge(g.find("how", "are", 1)) == g.find("how", "how", 4));
		CHECK(g.erase_edge(g.find("how", "how", 4)) == g.find("you", "how", 3));
		CHECK(g.erase_edge(g.find("you", "how", 3)) == g.end());
		CHECK(g.erase_edge(g.find("are", "you", 2)) == g.end());
	}
}

TEST_CASE("Erase edge (iterator, iterator)") {
	auto g = gdwg::graph<std::string, int>{"how", "are", "you"};
	SECTION("erase single edge") {
		g.insert_edge("how", "are", 1);
		g.insert_edge("are", "you", 2);
		CHECK(g.erase_edge(g.begin(), g.find("how", "are", 2)) == g.find("how", "are", 1));
		CHECK(g.erase_edge(g.begin(), g.end()) == g.end());
	}

	SECTION("Erase multiple edges") {
		g.insert_edge("how", "are", 1);
		g.insert_edge("are", "you", 2);
		g.insert_edge("you", "how", 3);
		g.insert_edge("how", "how", 4);
		CHECK(g.erase_edge(g.find("are", "you", 2), g.find("how", "how", 4)) == g.find("how", "how", 4));
		CHECK(g.erase_edge(g.begin(), g.end()) == g.end());
	}
}

TEST_CASE("Clear") {
	SECTION("Empty graph") {
		auto g = gdwg::graph<int, int>{};
		CHECK(g.empty());
		g.clear();
		CHECK(g.empty());
	}

	SECTION("Graph with nodes") {
		auto g = gdwg::graph<std::string, int>{};
		g.insert_node("hello");
		g.insert_node("how");
		g.insert_node("are");
		g.insert_node("you?");
		CHECK(!g.empty());
		g.clear();
		CHECK(g.empty());
	}

	SECTION("Graph with nodes and edges") {
		auto g = gdwg::graph<std::string, int>{};
		g.insert_node("hello");
		g.insert_node("how");
		g.insert_node("are");
		g.insert_node("you?");
		g.insert_edge("hello", "how", 5);
		g.insert_edge("hello", "are", 8);
		g.insert_edge("how", "you?", 1);
		CHECK(!g.empty());
		g.clear();
		CHECK(g.empty());
	}
}