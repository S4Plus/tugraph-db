﻿/**
 * Copyright 2024 AntGroup CO., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */

//
// Created by wt on 6/12/18.
//
#pragma once

#include <vector>
#include <unordered_map>
#include "cypher/execution_plan/ops/op.h"
#include "cypher/filter/filter.h"
#include "cypher/graph/graph.h"
#include "cypher/parser/clause.h"
#include "lgraph/lgraph.h"

namespace lgraph {
class StateMachine;
}

namespace cypher {

class ExecutionPlan {
    // global member
    parser::CmdType _cmd_type = parser::CmdType::QUERY;
    bool _read_only = true;
    OpBase *_root = nullptr;
    // resultset of CURRENT part being built, TODO(anyone) alloc for every part
    ResultInfo _result_info;
    // query parts local member
    std::vector<PatternGraph> _pattern_graphs;
    std::string _view_path;
    // std::map<std::string,PatternGraph*> _view_pattern_graphs;
    std::vector<std::pair<std::string, PatternGraph*>> _view_pattern_graphs;
    // std::vector<PatternGraph*> _view_pattern_graphs;
    std::string cypher_query_="";
    std::vector<std::vector<std::string>> id_seek_variables;
    bool _is_view_maintenance = false;
    bool _is_create_view=false;
    bool _is_optimize=false;
    bool _is_profile=false;
    size_t _db_hit=0;

    void _AddScanOp(const parser::QueryPart &part, const SymbolTable *sym_tab, Node *node,
                    std::vector<OpBase *> &ops, bool skip_arg_op);

    void _BuildArgument(const parser::QueryPart &part, PatternGraph &pattern_graph, OpBase *&root);

    void _BuildStandaloneCallOp(const parser::QueryPart &part, const PatternGraph &pattern_graph,
                                OpBase *&root);

    void _BuildExpandOps(const parser::QueryPart &part, PatternGraph &pattern_graph, OpBase *&root, int part_id);

    void _BuildUnwindOp(const parser::QueryPart &part, const PatternGraph &pattern_graph,
                        OpBase *&root);

    void _BuildInQueryCallOp(const parser::QueryPart &part, const PatternGraph &pattern_graph,
                             OpBase *&root);

    void _BuildCreateOp(const parser::QueryPart &part, PatternGraph &pattern_graph, OpBase *&root);

    void _BuildMergeOp(const parser::QueryPart &part, PatternGraph &pattern_graph, OpBase *&root);

    void _BuildDeleteOp(const parser::QueryPart &part, PatternGraph &pattern_graph, OpBase *&root);

    void _BuildSetOp(const parser::QueryPart &part, PatternGraph &pattern_graph, OpBase *&root);

    void _BuildRemoveOp(const parser::QueryPart &part, PatternGraph &pattern_graph, OpBase *&root);

    void _BuildReturnOps(const parser::QueryPart &part, const PatternGraph &pattern_graph,
                         OpBase *&root);

    void _BuildWithOps(const parser::QueryPart &part, const PatternGraph &pattern_graph,
                       OpBase *&root);

    void _BuildClause(const parser::Clause &clause, const parser::QueryPart &part,
                      PatternGraph &pattern_graph, OpBase *&root, int part_id);

    void _PlaceFilterOps(const parser::QueryPart &part, OpBase *&root);

    void _PlaceFilter(std::shared_ptr<lgraph::Filter> f, OpBase *&root);

    bool _PlaceFilterToNode(std::shared_ptr<lgraph::Filter> &f, OpBase *node);

    void _MergeFilter(OpBase *&root);

    bool _WorkWithoutTransaction(const parser::SglQuery &stmt) const;

    void GetViewPatternGraphs(cypher::RTContext *ctx);

    DISABLE_COPY(ExecutionPlan);
    DISABLE_MOVE(ExecutionPlan);

 public:

    ExecutionPlan() = default;

    ExecutionPlan &Clone(const ExecutionPlan &rhs);

    ~ExecutionPlan();

    OpBase *BuildPart(const parser::QueryPart &part, int part_id);

    OpBase *BuildSgl(const parser::SglQuery &stmt, size_t parts_offset);

    void Build(const std::vector<parser::SglQuery> &stmt, parser::CmdType cmd,
               cypher::RTContext *ctx);

    void Build(const std::vector<parser::SglQuery> &stmt, parser::CmdType cmd,
               cypher::RTContext *ctx, bool profile);

    void Validate(cypher::RTContext *ctx);

    void Reset();

    const ResultInfo &GetResultInfo() const;

    OpBase *Root() { return _root; }

    bool ReadOnly() const { return _read_only; }

    parser::CmdType CommandType() const { return _cmd_type; }

    int Execute(RTContext *ctx);

    int ExecuteWithoutNewTxn(RTContext *ctx);

    std::string DumpPlan(int indent, bool statistics) const;

    std::string DumpGraph() const;  // dump pattern graph

    const std::vector<PatternGraph>& GetPatternGraphs() const { return _pattern_graphs; }

    std::string data_dir="";
    void SetMaintenance(bool is_view_maintenance) { _is_view_maintenance = is_view_maintenance; }
    void SetOptimize(bool is_optimize) { _is_optimize = is_optimize; }
    void SetCreateView(bool is_create_view) { _is_create_view = is_create_view; }
    void SetCypherQuery(std::string cypher_query) {cypher_query_=cypher_query;}
    size_t GetDBHit(){return _db_hit;}
    size_t CalculateDBHit(OpBase* root);
};
}  // namespace cypher