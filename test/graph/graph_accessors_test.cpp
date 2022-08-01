#include "gdwg/graph.hpp"
#include <catch2/catch.hpp>
#include <string>

TEST_CASE("Accessors") {
	auto g = gdwg::graph<std::string, int>{"how", "are", "you"};
	g.insert_edge("how", "are", 1);
	g.insert_edge("how", "you", 2);
	g.insert_edge("are", "you", 3);
	auto const const_g = g;

	SECTION("Is node") {
		CHECK(g.is_node("how"));
		CHECK(g.is_node("are"));
		CHECK(g.is_node("you"));
		CHECK_FALSE(g.is_node("?"));

		CHECK(const_g.is_node("how"));
		CHECK(const_g.is_node("are"));
		CHECK(const_g.is_node("you"));
		CHECK_FALSE(const_g.is_node("?"));
	}
	SECTION("Empty") {
		CHECK(gdwg::graph<std::string, int>{}.empty());
		CHECK_FALSE(g.empty());
		CHECK_FALSE(const_g.empty());
	}
	SECTION("Is connected") {
		SECTION("Connect or not") {}
		SECTION("Exception: either src or dst node does not exist") {}
	}
	SECTION("Nodes in vector") {
		CHECK(gdwg::graph<std::string, int>{}.nodes().empty());
		CHECK(g.nodes() == std::vector<std::string>{"are", "how", "you"});
		CHECK(const_g.nodes() == std::vector<std::string>{"are", "how", "you"});
	}
	SECTION("Weights in vector") {
		SECTION("No edges") {
			CHECK(g.weights("you", "you").empty());
			CHECK(const_g.weights("you", "you").empty());
		}

		SECTION("Multiple edges") {
			g.insert_edge("how", "you", 6);
			g.insert_edge("how", "you", 4);
			g.insert_edge("how", "you", 5);
			CHECK(g.weights("how", "you") == std::vector<int>{2, 4, 5, 6});
			auto const const_g1 = g;
			CHECK(const_g1.weights("how", "you") == std::vector<int>{2, 4, 5, 6});
		}

		SECTION("Exception: either src or dst node does not exist") {
			CHECK_THROWS_MATCHES(g.weights("hello", "how"),
			                     std::runtime_error,
			                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::weights if "
			                                              "src or dst node don't exist in the graph"));

			// dst does not exist
			CHECK_THROWS_MATCHES(g.weights("how", "hello"),
			                     std::runtime_error,
			                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::weights if "
			                                              "src or dst node don't exist in the graph"));

			// both src and dst does not exist
			CHECK_THROWS_MATCHES(g.weights("hello", "world"),
			                     std::runtime_error,
			                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::weights if "
			                                              "src or dst node don't exist in the graph"));
		}
	}
	SECTION("Find edge") {
		SECTION("Edge not exist") {
			CHECK(g.find("how", "are", 2) == g.end());
			CHECK(g.find("how", "how", 2) == g.end());
			CHECK(const_g.find("how", "are", 2) == const_g.end());
			CHECK(const_g.find("how", "how", 2) == const_g.end());
		}

		SECTION("Edge exist") {
			auto it = g.find("how", "you", 2);
			CHECK((*it).from == "how");
			CHECK((*it).to == "you");
			CHECK((*it).weight == 2);
			auto const_it = g.find("how", "you", 2);
			CHECK((*const_it).from == "how");
			CHECK((*const_it).to == "you");
			CHECK((*const_it).weight == 2);
		}
	}
	SECTION("Connections") {
		SECTION("No connections") {
			CHECK(g.connections("you").empty());
			CHECK(const_g.connections("you").empty());
		}

		SECTION("Multiple connections in correct order") {
			g.insert_edge("how", "you", 6);
			g.insert_edge("how", "you", 4);
			g.insert_edge("how", "how", 5);
			CHECK(g.connections("how") == std::vector<std::string>{"are", "how", "you"});
		}

		SECTION("Exception: src node does not exist") {
			CHECK_THROWS_MATCHES(g.connections("hi"),
			                     std::runtime_error,
			                     Catch::Matchers::Message("Cannot call gdwg::graph<N, E>::connections "
			                                              "if src doesn't exist in the graph"));
		}
	}
}
