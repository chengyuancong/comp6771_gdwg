/* rationale and approach

Constructor && Assignement
--------------------------
Construct graph by different constructor and then use accessor functions to check if the graph
is constructed correctly (Assuming some accessor functions are correctly implemented).

Since the spec requires that graph has a own copy of the nodes value and edge weight,
we also test this by inserting nodes and check if the nodes value is still available after the
value goes out of scope.

For assignment operators, we need to ensure the two relevant objects are in the state as
specified (e.g. moved-from object is in empty state).

Modifier
--------
To see if the graph changes as expected, we use accessors
function to check if the graph is in correct state.

We also need to check if exceptions are thrown as expected.

All tests follow steps as using modifier to make changes and then
use accessors to check if the changeshave been made successfully.

Accessor
--------
To test accessors, just construct the graph, make some
modifications and see if a accessor gives the expected state.

This assumes constructors and modifiers are correctly implemented.

We also need to check if exceptions are thrown as expected.

Iterator
--------
To test if ++ -- works on iterators, we need to dereference
the returned iterator and see if it give the right result.

For iterator comparasion, we check if iterator pointing to the same edge are equal.

Iterator begin and end are tested to ensure they points to correct edge.

The returned value_type is just a copy rather than the actual data in
the graph, so all the above functions should work on both const and non-const graph.

Extractor && comparasion
------------------------
For comparasion, check if the correct boolean result is returned.
For extractor, check if the correct string is printed as expected.

*/

#include "gdwg/graph.hpp"
#include <catch2/catch.hpp>

TEST_CASE("Default constructor") {
	auto g = gdwg::graph<int, std::string>{};
	CHECK(g.empty());
}

TEST_CASE("Initializer list constructor") {
	SECTION("Empty list") {
		auto g = gdwg::graph<int, std::string>{};
		CHECK(g.empty());
	}
	SECTION("int list") {
		auto g = gdwg::graph<int, std::string>{1, 2, 3};
		CHECK(g.is_node(1));
		CHECK(g.is_node(2));
		CHECK(g.is_node(3));
	}
	SECTION("std::string list") {
		auto g = gdwg::graph<std::string, int>{"hello", "how", "are"};
		CHECK(g.is_node("hello"));
		CHECK(g.is_node("how"));
		CHECK(g.is_node("are"));
	}
	SECTION("std::vector<int> list") {
		auto v1 = std::vector<int>{1, 3, 2};
		auto v2 = std::vector<int>{4, 6};
		auto v3 = std::vector<int>{3, 5, 7, 9};
		auto v4 = std::vector<int>{};
		auto v5 = std::vector<int>{-1, -2, -3};
		auto v6 = std::vector<int>{4, 5, 8};
		auto g = gdwg::graph<std::vector<int>, int>{v1, v2, v3, v4, v5, v6};
		CHECK(g.is_node(v1));
		CHECK(g.is_node(v2));
		CHECK(g.is_node(v3));
		CHECK(g.is_node(v4));
		CHECK(g.is_node(v5));
		CHECK(g.is_node(v6));
	}
}

TEST_CASE("InputIt constructor") {
	SECTION("std::string set") {
		auto v = std::set<std::string>{"hello", "how", "are"};
		auto g = gdwg::graph<std::string, int>(v.begin(), v.end());
		CHECK(g.is_node("hello"));
		CHECK(g.is_node("how"));
		CHECK(g.is_node("are"));
	}

	SECTION("int vector") {
		auto v = std::vector<int>{1, 3, 2};
		auto g = gdwg::graph<int, std::string>(v.begin(), v.end());
		CHECK(g.is_node(1));
		CHECK(g.is_node(3));
		CHECK(g.is_node(2));
	}
}

TEST_CASE("Move constructor") {
	// move-from graph
	auto g = gdwg::graph<std::string, int>{"hello", "how", "are"};
	CHECK(g.is_node("hello"));
	CHECK(g.is_node("how"));
	CHECK(g.is_node("are"));
	g.insert_edge("hello", "how", 5);
	g.insert_edge("hello", "are", 8);
	g.insert_edge("are", "how", 2);
	g.insert_edge("are", "are", 4);
	CHECK(g.is_connected("hello", "how"));
	CHECK(g.is_connected("hello", "are"));
	CHECK(g.is_connected("are", "how"));
	CHECK(g.is_connected("are", "are"));

	// move-to graph
	auto g2 = std::move(g);
	CHECK(g2.is_node("hello"));
	CHECK(g2.is_node("how"));
	CHECK(g2.is_node("are"));
	CHECK(g2.is_connected("hello", "how"));
	CHECK(g2.is_connected("hello", "are"));
	CHECK(g2.is_connected("are", "how"));
	CHECK(g2.is_connected("are", "are"));
	// NOLINTNEXTLINE(bugprone-use-after-move)
	CHECK(g.empty());
}

TEST_CASE("Move assignment") {
	// move-from graph
	auto g = gdwg::graph<std::string, int>{"hello", "how", "are"};
	CHECK(g.is_node("hello"));
	CHECK(g.is_node("how"));
	CHECK(g.is_node("are"));
	g.insert_edge("hello", "how", 5);
	g.insert_edge("hello", "are", 8);
	g.insert_edge("are", "how", 2);
	g.insert_edge("are", "are", 4);
	CHECK(g.is_connected("hello", "how"));
	CHECK(g.is_connected("hello", "are"));
	CHECK(g.is_connected("are", "how"));
	CHECK(g.is_connected("are", "are"));

	// move-to graph
	auto g2 = gdwg::graph<std::string, int>{"a", "b", "c"};
	CHECK(g2.is_node("a"));
	CHECK(g2.is_node("b"));
	CHECK(g2.is_node("c"));
	g2.insert_edge("a", "b", 5);
	g2.insert_edge("a", "c", 8);
	g2.insert_edge("b", "c", 2);
	g2.insert_edge("b", "b", 4);
	CHECK(g2.is_connected("a", "b"));
	CHECK(g2.is_connected("a", "c"));
	CHECK(g2.is_connected("b", "c"));
	CHECK(g2.is_connected("b", "b"));

	// move g to g2
	g2 = std::move(g);
	CHECK(g2.is_node("hello"));
	CHECK(g2.is_node("how"));
	CHECK(g2.is_node("are"));
	CHECK(g2.is_connected("hello", "how"));
	CHECK(g2.is_connected("hello", "are"));
	CHECK(g2.is_connected("are", "how"));
	CHECK(g2.is_connected("are", "are"));
	// NOLINTNEXTLINE(bugprone-use-after-move)
	CHECK(g.empty());
}

TEST_CASE("Copy constructor") {
	// original graph
	auto g = gdwg::graph<std::string, int>{"hello", "how", "are"};
	CHECK(g.is_node("hello"));
	CHECK(g.is_node("how"));
	CHECK(g.is_node("are"));
	g.insert_edge("hello", "how", 5);
	g.insert_edge("hello", "are", 8);
	g.insert_edge("are", "how", 2);
	g.insert_edge("are", "are", 4);
	CHECK(g.is_connected("hello", "how"));
	CHECK(g.is_connected("hello", "are"));
	CHECK(g.is_connected("are", "how"));
	CHECK(g.is_connected("are", "are"));

	// new graph
	auto g2 = g;
	CHECK(g2.is_node("hello"));
	CHECK(g2.is_node("how"));
	CHECK(g2.is_node("are"));
	CHECK(g2.is_connected("hello", "how"));
	CHECK(g2.is_connected("hello", "are"));
	CHECK(g2.is_connected("are", "how"));
	CHECK(g2.is_connected("are", "are"));

	// change g does not change g2
	g.insert_node("good");
	g.insert_edge("how", "are", 9);
	CHECK(g.is_node("good"));
	CHECK(g.is_connected("how", "are"));
	CHECK_FALSE(g2.is_node("good"));
	CHECK_FALSE(g2.is_connected("how", "are"));
}

TEST_CASE("Copy assignment") {
	// original graph
	auto g = gdwg::graph<std::string, int>{"hello", "how", "are"};
	CHECK(g.is_node("hello"));
	CHECK(g.is_node("how"));
	CHECK(g.is_node("are"));
	g.insert_edge("hello", "how", 5);
	g.insert_edge("hello", "are", 8);
	g.insert_edge("are", "how", 2);
	g.insert_edge("are", "are", 4);
	CHECK(g.is_connected("hello", "how"));
	CHECK(g.is_connected("hello", "are"));
	CHECK(g.is_connected("are", "how"));
	CHECK(g.is_connected("are", "are"));

	// new graph
	auto g2 = gdwg::graph<std::string, int>{"a", "b", "c"};
	CHECK(g2.is_node("a"));
	CHECK(g2.is_node("b"));
	CHECK(g2.is_node("c"));
	g2.insert_edge("a", "b", 5);
	g2.insert_edge("a", "c", 8);
	g2.insert_edge("b", "c", 2);
	g2.insert_edge("b", "b", 4);
	CHECK(g2.is_connected("a", "b"));
	CHECK(g2.is_connected("a", "c"));
	CHECK(g2.is_connected("b", "c"));
	CHECK(g2.is_connected("b", "b"));

	// copy assign g to g2
	g2 = g;
	CHECK(g2.is_node("hello"));
	CHECK(g2.is_node("how"));
	CHECK(g2.is_node("are"));
	CHECK(g2.is_connected("hello", "how"));
	CHECK(g2.is_connected("hello", "are"));
	CHECK(g2.is_connected("are", "how"));
	CHECK(g2.is_connected("are", "are"));

	// change g does not change g2
	g.insert_node("good");
	g.insert_edge("how", "are", 9);
	CHECK(g.is_node("good"));
	CHECK(g.is_connected("how", "are"));
	CHECK_FALSE(g2.is_node("good"));
	CHECK_FALSE(g2.is_connected("how", "are"));
}
