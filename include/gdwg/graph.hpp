#ifndef GDWG_GRAPH_HPP
#define GDWG_GRAPH_HPP

#include <algorithm>
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <vector>

// This will not compile straight away
namespace gdwg {
	template<typename N, typename E>
	class graph {
	public:
		struct value_type {
			N from;
			N to;
			E weight;
			value_type(N const& from, N const& to, E const& weight)
			: from{from}
			, to{to}
			, weight{weight} {};
			~value_type() = default;
		};

	private:
		struct edge {
			N* src;
			N* dest;
			std::unique_ptr<E> weight;

			auto operator==(edge const& other) const -> bool {
				return std::tie(*src, *dest, *weight) == std::tie(*other.src, *other.dest, *other.weight);
			};

			// Constructor
			edge(N* src, N* dest, E const& weight)
			: src{src}
			, dest{dest}
			, weight{std::make_unique<E>(weight)} {};

			// Rule of 5
			edge(edge const&& orig) noexcept = default;
			auto operator=(edge&& orig) noexcept -> edge& = default;
			edge(edge const& orig) = delete;
			auto operator=(edge const& orig) -> edge& = delete;
			~edge() = default;
		};

		struct edge_cmp {
			using is_transparent = std::true_type;
			auto operator()(edge const& lhs, edge const& rhs) const -> bool {
				return std::tie(*lhs.src, *lhs.dest, *lhs.weight)
				       < std::tie(*rhs.src, *rhs.dest, *rhs.weight);
			};

			auto operator()(value_type const& lhs, edge const& rhs) const -> bool {
				return std::tie(lhs.from, lhs.to, lhs.weight)
				       < std::tie(*rhs.src, *rhs.dest, *rhs.weight);
			};

			auto operator()(edge const& lhs, value_type const& rhs) const -> bool {
				return std::tie(*lhs.src, *lhs.dest, *lhs.weight)
				       < std::tie(rhs.from, rhs.to, rhs.weight);
			};
		};

		struct node {
			std::unique_ptr<N> value;

			auto operator==(node const& other) const -> bool {
				return *value == *other.value;
			};

			// Constructor
			explicit node(N const& value)
			: value{std::make_unique<N>(value)} {};

			// Rule of 5
			node(node const&& orig) noexcept = default;
			auto operator=(node&& orig) noexcept -> node& = default;
			node(node const& orig) = delete;
			auto operator=(node const& orig) -> node& = delete;
			~node() = default;
		};

		struct node_cmp {
			using is_transparent = std::true_type;
			auto operator()(node const& lhs, node const& rhs) const -> bool {
				return *lhs.value < *rhs.value;
			};

			auto operator()(node const& lhs, N const& rhs) const -> bool {
				return *lhs.value < rhs;
			};

			auto operator()(N const& lhs, node const& rhs) const -> bool {
				return lhs < *rhs.value;
			};
		};

		std::set<node, node_cmp> nodes_;
		std::set<edge, edge_cmp> edges_;

	public:
		// Constructors
		graph() = default;

		graph(std::initializer_list<N> il)
		: graph(il.begin(), il.end()){};

		template<typename InputIt>
		graph(InputIt first, InputIt last) {
			std::for_each(first, last, [&](N const& n) { insert_node(n); });
		};

		// Move constructor
		graph(graph&& orig) noexcept = default;

		// Move	assignment
		auto operator=(graph&& orig) noexcept -> graph& = default;

		// Copy constructor
		graph(graph const& orig) {
			std::for_each(orig.nodes_.begin(), orig.nodes_.end(), [&](node const& n) {
				insert_node(*n.value);
			});
			std::for_each(orig.edges_.begin(), orig.edges_.end(), [&](edge const& e) {
				insert_edge(*e.src, *e.dest, *e.weight);
			});
		};

		// Copy assignment
		auto operator=(graph const& orig) -> graph& {
			if (this != &orig) {
				edges_.clear();
				nodes_.clear();
				std::for_each(orig.nodes_.begin(), orig.nodes_.end(), [&](node const& n) {
					insert_node(*n.value);
				});
				std::for_each(orig.edges_.begin(), orig.edges_.end(), [&](edge const& e) {
					insert_edge(*e.src, *e.dest, *e.weight);
				});
			};
			return *this;
		};

		// Destructor
		~graph() = default;

		// Iterator
		class iterator {
		public:
			using value_type = graph<N, E>::value_type;
			using reference = value_type;
			using pointer = void;
			using difference_type = std::ptrdiff_t;
			using iterator_category = std::bidirectional_iterator_tag;

			// Iterator constructor
			iterator() = default;

			// Iterator source
			auto operator*() -> reference {
				return value_type{*(itor_->src), *(itor_->dest), *(itor_->weight)};
			};

			// Iterator traversal
			auto operator++() -> iterator& {
				++itor_;
				return *this;
			};

			auto operator++(int) -> iterator {
				auto tmp = *this;
				++(*this);
				return tmp;
			};

			auto operator--() -> iterator& {
				--itor_;
				return *this;
			};

			auto operator--(int) -> iterator {
				auto tmp = *this;
				--(*this);
				return tmp;
			};

			// Iterator comparison
			auto operator==(iterator const& other) const -> bool {
				return itor_ == other.itor_;
			};

		private:
			using edge_itor = typename std::set<edge, edge_cmp>::iterator;
			edge_itor itor_;

			explicit iterator(edge_itor itor)
			: itor_{itor} {};

			friend class graph<N, E>;
		};

		// Modifiers
		auto insert_node(N const& value) -> bool {
			return nodes_.emplace(value).second;
		};

		auto insert_edge(N const& src, N const& dest, E const& weight) -> bool {
			auto src_itor = nodes_.find(src);
			auto dest_itor = nodes_.find(dest);
			if (not(src_itor != nodes_.end() and dest_itor != nodes_.end())) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::insert_edge "
				                         "when either src or dst node does not exist");
			};
			return edges_.emplace(src_itor->value.get(), dest_itor->value.get(), weight).second;
		};

		auto replace_node(N const& old_data, N const& new_data) -> bool {
			auto old_itor = nodes_.find(old_data);
			if (old_itor == nodes_.end()) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::replace_node "
				                         "on a node that doesn't exist");
			};
			if (is_node(new_data)) {
				return false;
			};
			nodes_.emplace(new_data);
			auto old_edges = std::vector<value_type>{};
			std::for_each(edges_.begin(), edges_.end(), [&](edge const& e) {
				if (e.src == old_itor->value.get() || e.dest == old_itor->value.get()) {
					old_edges.push_back(value_type{*e.src, *e.dest, *e.weight});
				};
			});
			std::for_each(old_edges.begin(), old_edges.end(), [&](value_type const& e) {
				auto const& new_src = e.from == old_data ? new_data : e.from;
				auto const& new_dest = e.to == old_data ? new_data : e.to;
				insert_edge(new_src, new_dest, e.weight);
				erase_edge(e.from, e.to, e.weight);
			});
			nodes_.erase(old_itor);
			return true;
		};

		auto merge_replace_node(N const& old_data, N const& new_data) -> void {
			auto old_itor = nodes_.find(old_data);
			auto new_itor = nodes_.find(new_data);
			if (old_itor == nodes_.end() || new_itor == nodes_.end()) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::merge_replace_node "
				                         "on old or new data if they don't exist in the graph");
			};
			if (old_itor == new_itor) {
				return;
			};
			auto edges = std::vector<value_type>{};
			std::for_each(edges_.begin(), edges_.end(), [&](edge const& e) {
				if (e.src == old_itor->value.get() || e.dest == old_itor->value.get()) {
					edges.push_back(value_type{*e.src, *e.dest, *e.weight});
				};
			});
			std::for_each(edges.begin(), edges.end(), [&](value_type const& e) {
				auto const& new_src = e.from == old_data ? new_data : e.from;
				auto const& new_dest = e.to == old_data ? new_data : e.to;
				if (edges_.find(value_type{new_src, new_dest, e.weight}) == edges_.end()) {
					insert_edge(new_src, new_dest, e.weight);
				};
				erase_edge(e.from, e.to, e.weight);
			});
			nodes_.erase(old_itor);
		};

		auto erase_node(N const& value) -> bool {
			auto itor = nodes_.find(value);
			if (itor == nodes_.end()) {
				return false;
			};
			auto edges = std::vector<value_type>{};
			std::for_each(edges_.begin(), edges_.end(), [&](edge const& e) {
				if (e.src == itor->value.get() || e.dest == itor->value.get()) {
					edges.push_back(value_type{*e.src, *e.dest, *e.weight});
				};
			});
			std::for_each(edges.begin(), edges.end(), [&](value_type const& e) {
				erase_edge(e.from, e.to, e.weight);
			});
			nodes_.erase(itor);
			return true;
		};

		auto erase_edge(N const& src, N const& dest, E const& weight) -> bool {
			auto src_itor = nodes_.find(src);
			auto dest_itor = nodes_.find(dest);
			if (src_itor == nodes_.end() or dest_itor == nodes_.end()) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::erase_edge "
				                         "on src or dst if they don't exist in the graph");
			};
			auto edge_itor = edges_.find(value_type(src, dest, weight));
			if (edge_itor == edges_.end()) {
				return false;
			};
			edges_.erase(edge_itor);
			return true;
		};

		auto erase_edge(iterator i) -> iterator {
			return iterator{edges_.erase(i.itor_)};
		};
		auto erase_edge(iterator i, iterator s) -> iterator {
			return iterator{edges_.erase(i.itor_, s.itor_)};
		};

		auto clear() noexcept -> void {
			nodes_.clear();
			edges_.clear();
		};

		// Accessors
		[[nodiscard]] auto is_node(N const& value) const -> bool {
			return nodes_.find(value) != nodes_.end();
		};

		[[nodiscard]] auto empty() const noexcept -> bool {
			return nodes_.empty();
		};

		[[nodiscard]] auto is_connected(N const& src, N const& dest) const -> bool {
			auto src_itor = nodes_.find(src);
			auto dest_itor = nodes_.find(dest);
			if (src_itor == nodes_.end() or dest_itor == nodes_.end()) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::is_connected "
				                         "if src or dst node don't exist in the graph");
			};
			return std::find_if(edges_.begin(),
			                    edges_.end(),
			                    [&src_itor, &dest_itor](auto const& e) {
				                    return e.src == src_itor->value.get()
				                           and e.dest == dest_itor->value.get();
			                    })
			       != edges_.end();
		};

		[[nodiscard]] auto nodes() const -> std::vector<N> {
			auto nodes = std::vector<N>();
			std::transform(nodes_.begin(), nodes_.end(), std::back_inserter(nodes), [](auto const& n) {
				return *n.value;
			});
			return nodes;
		};

		[[nodiscard]] auto weights(N const& src, N const& dest) const -> std::vector<E> {
			auto src_itor = nodes_.find(src);
			auto dest_itor = nodes_.find(dest);
			if (src_itor == nodes_.end() or dest_itor == nodes_.end()) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::weights "
				                         "if src or dst node don't exist in the graph");
			};
			auto weights = std::vector<E>();
			std::for_each(edges_.begin(), edges_.end(), [&weights, &src_itor, &dest_itor](auto const& e) {
				if (e.src == src_itor->value.get() and e.dest == dest_itor->value.get()) {
					weights.push_back(*e.weight);
				};
			});
			return weights;
		};

		[[nodiscard]] auto find(N const& src, N const& dest, E const& weight) const -> iterator {
			return iterator{edges_.find(value_type(src, dest, weight))};
		};

		[[nodiscard]] auto connections(N const& src) const -> std::vector<N> {
			auto src_itor = nodes_.find(src);
			if (src_itor == nodes_.end()) {
				throw std::runtime_error("Cannot call gdwg::graph<N, E>::connections "
				                         "if src doesn't exist in the graph");
			};
			auto connections = std::set<N>();
			std::for_each(edges_.begin(), edges_.end(), [&connections, &src_itor](auto const& e) {
				if (e.src == src_itor->value.get()) {
					connections.insert(*e.dest);
				};
			});
			return std::vector<N>(connections.begin(), connections.end());
		};

		// Iterator access
		[[nodiscard]] auto begin() const -> iterator {
			return iterator{edges_.begin()};
		};

		[[nodiscard]] auto end() const -> iterator {
			return iterator{edges_.end()};
		};

		// Comparisons
		[[nodiscard]] auto operator==(graph const& other) const noexcept -> bool = default;

		// Extractor
		friend auto operator<<(std::ostream& os, graph const& g) -> std::ostream& {
			auto oss = std::ostringstream{};
			std::for_each(g.nodes_.begin(), g.nodes_.end(), [&oss, &g](auto const& n) {
				oss << *n.value << " (\n";
				std::for_each(g.edges_.begin(), g.edges_.end(), [&oss, &n](auto const& e) {
					if (e.src == n.value.get()) {
						oss << "  " << *e.dest << " | " << *e.weight << "\n";
					};
				});
				oss << ")\n";
			});
			return os << oss.str();
		};
	};
} // namespace gdwg
#endif // GDWG_GRAPH_HPP
