#ifndef MEANWHILE_IN_THE_SEVENTH_GALAXY_THIRD_VARIANT_DFS_H
#define MEANWHILE_IN_THE_SEVENTH_GALAXY_THIRD_VARIANT_DFS_H

#include "type_list.h"
#include "value_list.h"
#include "graph.h"

template<size_t N1, size_t N2>
struct Equal {
  static constexpr bool value = false;
};

template<size_t N1>
struct Equal<N1, N1> {
  static constexpr bool value = true;
};

template<bool IsVisited, typename Graph, size_t CurVer, size_t TargetVer, typename Visited>
struct PathExistsImpl {
};

template<bool SolutionFound, typename Graph, size_t TargetVerIdx, typename Visited, typename Edges>
struct VisitEdges {
};

template<typename Graph, size_t TargetVerIdx, typename Visited, typename... T>
class VisitEdges<false, Graph, TargetVerIdx, Visited, TypeList<T...>> {
  using CurEdge = typename TypeList<T...>::Head;
  static constexpr size_t to = CurEdge::to;
  static constexpr bool isVisited = Get<to, Visited>::value;

  using VerResult = PathExistsImpl<isVisited, Graph, to, TargetVerIdx, Visited>;
  using RecursiveResult = VisitEdges<VerResult::result,
                                     Graph,
                                     TargetVerIdx,
                                     typename VerResult::ResultVisited,
                                     typename TypeList<T...>::Tail>;

 public:
  static constexpr bool result = VerResult::result || RecursiveResult::result;
  using ResultVisited = typename RecursiveResult::ResultVisited;
};

// stop when empty edges
template<typename Graph, size_t TargetVerIdx, typename Visited>
struct VisitEdges<false, Graph, TargetVerIdx, Visited, NullType> {
  static constexpr bool result = false;
  using ResultVisited = Visited;
};

// stop when solution found
template<typename Graph, size_t TargetVerIdx, typename Visited, typename... T>
struct VisitEdges<true, Graph, TargetVerIdx, Visited, TypeList<T...>> {
  static constexpr bool result = true;
  using ResultVisited = Visited;
};

// stop when solution found with empty Edges
template<typename Graph, size_t TargetVerIdx, typename Visited>
struct VisitEdges<true, Graph, TargetVerIdx, Visited, NullType> {
  static constexpr bool result = true;
  using ResultVisited = Visited;
};

template<typename Graph, size_t CurVerIdx, size_t TargetVerIdx, typename Visited>
class PathExistsImpl<false, Graph, CurVerIdx, TargetVerIdx, Visited> {
  using CurVisited = typename Set<CurVerIdx, true, Visited>::type;
  using CurVer = typename TypeAt<CurVerIdx, typename Graph::Vertices>::type;
  using CurEdges = typename CurVer::Edges;
  static constexpr bool curResult = Equal<CurVerIdx, TargetVerIdx>::value;

  using VisitedEdges = VisitEdges<curResult, Graph, TargetVerIdx, CurVisited, CurEdges>;

 public:
  static constexpr size_t result = curResult || VisitedEdges::result;
  using ResultVisited = typename VisitedEdges::ResultVisited;
};

// stop if this vertex is visited
template<typename Graph, size_t CurVerIdx, size_t TargetVerIdx, typename Visited>
struct PathExistsImpl<true, Graph, CurVerIdx, TargetVerIdx, Visited> {
  static constexpr bool result = false;
  using ResultVisited = Visited;
};

template<typename Graph, size_t start, size_t end>
struct PathExists {
  static constexpr size_t vertexCount = Length<typename Graph::Vertices>::value;
  using Visited = typename Construct<false, vertexCount>::type;

 public:
  static constexpr bool value = PathExistsImpl<false, Graph, start, end, Visited>::result;
};

template<size_t start, size_t end>
struct PathExists<Graph<>, start, end> {
 public:
  static constexpr bool value = false;
};

#endif /// MEANWHILE_IN_THE_SEVENTH_GALAXY_THIRD_VARIANT_DFS_H.
