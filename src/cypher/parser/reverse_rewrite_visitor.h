/**
 * Copyright 2022 AntGroup CO., Ltd.
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

// Generated from Lcypher.g4 by ANTLR 4.9.2

#pragma once

#include <cassert>
#include <tuple>
#include "antlr4-runtime/antlr4-runtime.h"
#include "parser/expression.h"
#include "parser/generated/LcypherVisitor.h"
#include "fma-common/utils.h"
#include "parser/clause.h"
#include "cypher/cypher_exception.h"
#include "procedure/procedure.h"
#include "core/defs.h"
#include "db/galaxy.h"

#if __APPLE__
#ifdef TRUE
#undef TRUE
#endif
#ifdef FALSE
#undef FALSE
#endif
#endif  // #if __APPLE__

namespace parser {

/**
 * This class provides an empty implementation of LcypherVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class ReverseRewriteVisitor : public LcypherVisitor {
    cypher::RTContext *ctx_;
    size_t curr_pattern_graph = 0;  // 在整个Cypher中的第几个pattern_graph
    std::string opt_query = "";
    const std::vector<cypher::PatternGraph> &pattern_graphs_;

    /* Anonymous entity are not in symbol table:
     * MATCH (n) RETURN exists((n)-->()-->())  */
    size_t _anonymous_idx = 0;

    enum _ClauseType : uint32_t {
        NA = 0x0,
        MATCH = 0x1,
        RETURN = 0x2,
        WITH = 0x4,
        UNWIND = 0x8,
        WHERE = 0x10,
        ORDERBY = 0x20,
        CREATE = 0x40,
        DELETE = 0x80,
        SET = 0x100,
        REMOVE = 0x200,
        MERGE = 0x400,
        INQUERYCALL = 0x800,
    } _curr_clause_type = NA;
    std::string GenAnonymousAlias(bool is_node) {
        std::string alias(ANONYMOUS);
        if (is_node) {
            alias.append("N").append(std::to_string(_anonymous_idx));
        } else {
            alias.append("R").append(std::to_string(_anonymous_idx));
        }
        _anonymous_idx++;
        return alias;
    }

 public:
    ReverseRewriteVisitor() = default;
    ReverseRewriteVisitor(cypher::RTContext *ctx, antlr4::tree::ParseTree *tree,
                             const std::vector<cypher::PatternGraph> &pattern_graphs)
        : ctx_(ctx), pattern_graphs_(pattern_graphs) {
        tree->accept(this);
    }


    std::string visitChildrenToString(antlr4::tree::ParseTree *node){
        std::string result;
        for (auto child : node->children) {
            std::string child_text;
            if (child->children.size() > 0)
                child_text = std::any_cast<std::string>(visit(child));
            else
                child_text = child->getText();
            result.append(child_text);
        }
        return result;
    }
    std::string GetOptQuery() const { return opt_query; }

    std::any visitOC_Cypher(LcypherParser::OC_CypherContext *ctx) override {
        opt_query = std::any_cast<std::string>(visit(ctx->oC_Statement()));
        return 0;
    }

    std::any visitOC_Statement(LcypherParser::OC_StatementContext *ctx) override {
        return std::any_cast<std::string>(visit(ctx->oC_Query()));
    }

    std::any visitOC_Query(LcypherParser::OC_QueryContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_View(LcypherParser::OC_ViewContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_Construct(LcypherParser::OC_ConstructContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_RegularQuery(LcypherParser::OC_RegularQueryContext *ctx) override {
        // reserve for single_queries
        _anonymous_idx = 0;
        return visitChildrenToString(ctx);
    }

    std::any visitOC_Union(LcypherParser::OC_UnionContext *ctx) override {
        _anonymous_idx = 0;
        curr_pattern_graph++;
        return visitChildrenToString(ctx);
    }

    std::any visitOC_SingleQuery(LcypherParser::OC_SingleQueryContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_SinglePartQuery(LcypherParser::OC_SinglePartQueryContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_MultiPartQuery(LcypherParser::OC_MultiPartQueryContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_UpdatingClause(LcypherParser::OC_UpdatingClauseContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_ReadingClause(LcypherParser::OC_ReadingClauseContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::string ReverseNode(const cypher::Node *node){
        std::string node_cypher="";
        node_cypher.append("(");
        if(!node->Alias().empty()){
            std::string name=node->Alias();
            if(name[0]=='@')name=name.substr(1);
            node_cypher.append(name);
        }
        if(!node->Label().empty()){
            node_cypher.append(":"+node->Label());
        }
        if(!node->Prop().Empty()){
            node_cypher.append("{");
            node_cypher.append(node->Prop().field+":");
            if(node->Prop().type==2){
                if(node->Prop().value.type==lgraph::FieldType::STRING){
                    node_cypher.append("'");
                }
                node_cypher.append(node->Prop().value.ToString());
                if(node->Prop().value.type==lgraph::FieldType::STRING){
                    node_cypher.append("'");
                }
            }
            else{
                node_cypher.append(node->Prop().value_alias);
            }
            node_cypher.append("}");
        }

        node_cypher.append(")");
        return node_cypher;
    }

    std::string ReverseRelp(const cypher::Relationship *relationship){
        std::string relp_cypher="";
        if(relationship->direction_==LEFT_TO_RIGHT)
                relp_cypher.append("-");
            else if(relationship->direction_==RIGHT_TO_LEFT)
                relp_cypher.append("<-");
            else
                relp_cypher.append("-");
            relp_cypher.append("[");
            if(!relationship->Alias().empty()){
                std::string name=relationship->Alias();
                if(name[0]=='@')name=name.substr(1);
                relp_cypher.append(name);
            }
            if(!relationship->Types().empty()){
                relp_cypher.append(":");
                int temp_index = 0;
                for(auto it:relationship->Types()){
                    if(temp_index>0)
                        relp_cypher.append("|");
                    relp_cypher.append(it);                  
                    temp_index++;
                }

            }
            if(relationship->VarLen())
            {
                relp_cypher.append("*").append(std::to_string(relationship->MinHop())).append("..");
                if(relationship->MaxHop()<128)
                    relp_cypher.append(std::to_string(relationship->MaxHop()));
            }

            // TODO: relationship Property
            relp_cypher.append("]");
            if(relationship->direction_==LEFT_TO_RIGHT)
                relp_cypher.append("->");
            else if(relationship->direction_==RIGHT_TO_LEFT)
                relp_cypher.append("-");
            else
                relp_cypher.append("-");
        return relp_cypher;
    }

    std::any visitOC_Match(LcypherParser::OC_MatchContext *ctx) override {
        _EnterClauseMATCH();
        std::string result=visitChildrenToString(ctx);
        _LeaveClauseMATCH();
        return result;
        //return visitChildrenToString(ctx);
    }

    std::any visitOC_Unwind(LcypherParser::OC_UnwindContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_Merge(LcypherParser::OC_MergeContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_MergeAction(LcypherParser::OC_MergeActionContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_Create(LcypherParser::OC_CreateContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_Set(LcypherParser::OC_SetContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_SetItem(LcypherParser::OC_SetItemContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_Delete(LcypherParser::OC_DeleteContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_Remove(LcypherParser::OC_RemoveContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_RemoveItem(LcypherParser::OC_RemoveItemContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_InQueryCall(LcypherParser::OC_InQueryCallContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_StandaloneCall(LcypherParser::OC_StandaloneCallContext *ctx) override {
        _anonymous_idx = 0;
        return visitChildrenToString(ctx);
    }

    std::any visitOC_YieldItems(LcypherParser::OC_YieldItemsContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_YieldItem(LcypherParser::OC_YieldItemContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_With(LcypherParser::OC_WithContext *ctx) override {
        std::string result = visitChildrenToString(ctx);
        curr_pattern_graph++;
        return result;
    }

    std::any visitOC_Return(LcypherParser::OC_ReturnContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_ReturnBody(LcypherParser::OC_ReturnBodyContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_ReturnItems(LcypherParser::OC_ReturnItemsContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_ReturnItem(LcypherParser::OC_ReturnItemContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_Order(LcypherParser::OC_OrderContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_Skip(LcypherParser::OC_SkipContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_Limit(LcypherParser::OC_LimitContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_SortItem(LcypherParser::OC_SortItemContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_Hint(LcypherParser::OC_HintContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_Where(LcypherParser::OC_WhereContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_Pattern(LcypherParser::OC_PatternContext *ctx) override {
        if(_InClauseMATCH()){
            int i=curr_pattern_graph;
            auto &pattern_graph=pattern_graphs_[i];
            std::string opti="";
            auto &relationships=pattern_graph.GetRelationships();
            std::vector<cypher::PatternGraph>::size_type j = 0;
            int temp_index=0;
            bool have_node=false;
            for(auto &node:pattern_graph.GetNodes()){
                if(node.derivation_!=cypher::Node::Derivation::MATCHED)continue;
                have_node=true;
                if(temp_index>0)opti.append(",");
                opti.append(ReverseNode(&node));
                temp_index++;
            }
            for(auto &relationship:relationships){
                if(relationship.derivation_!=cypher::Relationship::Derivation::MATCHED)continue;
                if(have_node){
                    opti.append(", ");
                    have_node=false;
                }
                cypher::NodeID lhs=relationship.Lhs();
                cypher::NodeID rhs=relationship.Rhs();
                auto &lnode=pattern_graph.GetNode(lhs);
                auto &rnode=pattern_graph.GetNode(rhs);
                opti.append("("+lnode.Alias()+")");
                // opti.append("(").append(lnode.Alias()).append(":").append(lnode.Label()).append(")");
                opti.append(ReverseRelp(&relationship));

                opti.append("("+rnode.Alias()+")");
                //   opti.append("(").append(rnode.Alias()).append(":").append(rnode.Label()).append(")");
                j++;
                if(j!=relationships.size())
                opti.append(",");
            }
            return opti;
        }
        else
        return visitChildrenToString(ctx);
    }

    std::any visitOC_PatternPart(LcypherParser::OC_PatternPartContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_AnonymousPatternPart(
        LcypherParser::OC_AnonymousPatternPartContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_PatternElement(LcypherParser::OC_PatternElementContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_NodePattern(LcypherParser::OC_NodePatternContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_PatternElementChain(LcypherParser::OC_PatternElementChainContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_RelationshipPattern(LcypherParser::OC_RelationshipPatternContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_RelationshipDetail(LcypherParser::OC_RelationshipDetailContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_Properties(LcypherParser::OC_PropertiesContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_RelationshipTypes(LcypherParser::OC_RelationshipTypesContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_NodeLabels(LcypherParser::OC_NodeLabelsContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_NodeLabel(LcypherParser::OC_NodeLabelContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_RangeLiteral(LcypherParser::OC_RangeLiteralContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_LabelName(LcypherParser::OC_LabelNameContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_RelTypeName(LcypherParser::OC_RelTypeNameContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_Expression(LcypherParser::OC_ExpressionContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_OrExpression(LcypherParser::OC_OrExpressionContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_XorExpression(LcypherParser::OC_XorExpressionContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_AndExpression(LcypherParser::OC_AndExpressionContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_NotExpression(LcypherParser::OC_NotExpressionContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_ComparisonExpression(
        LcypherParser::OC_ComparisonExpressionContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_AddOrSubtractExpression(
        LcypherParser::OC_AddOrSubtractExpressionContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_MultiplyDivideModuloExpression(
        LcypherParser::OC_MultiplyDivideModuloExpressionContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_PowerOfExpression(LcypherParser::OC_PowerOfExpressionContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_UnaryAddOrSubtractExpression(
        LcypherParser::OC_UnaryAddOrSubtractExpressionContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_StringListNullOperatorExpression(
        LcypherParser::OC_StringListNullOperatorExpressionContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_ListOperatorExpression(
        LcypherParser::OC_ListOperatorExpressionContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_StringOperatorExpression(
        LcypherParser::OC_StringOperatorExpressionContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_NullOperatorExpression(
        LcypherParser::OC_NullOperatorExpressionContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_PropertyOrLabelsExpression(
        LcypherParser::OC_PropertyOrLabelsExpressionContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_Atom(LcypherParser::OC_AtomContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_Literal(LcypherParser::OC_LiteralContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_BooleanLiteral(LcypherParser::OC_BooleanLiteralContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_ListLiteral(LcypherParser::OC_ListLiteralContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_PartialComparisonExpression(
        LcypherParser::OC_PartialComparisonExpressionContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_ParenthesizedExpression(
        LcypherParser::OC_ParenthesizedExpressionContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_RelationshipsPattern(
        LcypherParser::OC_RelationshipsPatternContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_FilterExpression(LcypherParser::OC_FilterExpressionContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_IdInColl(LcypherParser::OC_IdInCollContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_FunctionInvocation(LcypherParser::OC_FunctionInvocationContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_FunctionName(LcypherParser::OC_FunctionNameContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_ExplicitProcedureInvocation(
        LcypherParser::OC_ExplicitProcedureInvocationContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_ImplicitProcedureInvocation(
        LcypherParser::OC_ImplicitProcedureInvocationContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_ProcedureResultField(
        LcypherParser::OC_ProcedureResultFieldContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_ProcedureName(LcypherParser::OC_ProcedureNameContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_Namespace(LcypherParser::OC_NamespaceContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_ListComprehension(LcypherParser::OC_ListComprehensionContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_PatternComprehension(
        LcypherParser::OC_PatternComprehensionContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_PropertyLookup(LcypherParser::OC_PropertyLookupContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_CaseExpression(LcypherParser::OC_CaseExpressionContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_CaseAlternatives(LcypherParser::OC_CaseAlternativesContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_Variable(LcypherParser::OC_VariableContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_NumberLiteral(LcypherParser::OC_NumberLiteralContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_MapLiteral(LcypherParser::OC_MapLiteralContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_Parameter(LcypherParser::OC_ParameterContext *ctx) override {
        return ctx->getText();
    }

    std::any visitOC_PropertyExpression(LcypherParser::OC_PropertyExpressionContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_PropertyKeyName(LcypherParser::OC_PropertyKeyNameContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_IntegerLiteral(LcypherParser::OC_IntegerLiteralContext *ctx) override {
        return ctx->getText();
    }

    std::any visitOC_DoubleLiteral(LcypherParser::OC_DoubleLiteralContext *ctx) override {
        return ctx->getText();
    }

    std::any visitOC_SchemaName(LcypherParser::OC_SchemaNameContext *ctx) override {
        return visitChildrenToString(ctx);
    }

    std::any visitOC_ReservedWord(LcypherParser::OC_ReservedWordContext *ctx) override {
        return ctx->getText();
    }

    std::any visitOC_SymbolicName(LcypherParser::OC_SymbolicNameContext *ctx) override {
        return ctx->getText();
    }

    std::any visitOC_LeftArrowHead(LcypherParser::OC_LeftArrowHeadContext *ctx) override {
        return ctx->getText();
    }

    std::any visitOC_RightArrowHead(LcypherParser::OC_RightArrowHeadContext *ctx) override {
        return ctx->getText();
    }

    std::any visitOC_Dash(LcypherParser::OC_DashContext *ctx) override {
        return ctx->getText();
    }

#define CLAUSE_HELPER_FUNC(clause)                                                   \
    inline bool _InClause##clause() {                                                \
        return (_curr_clause_type & static_cast<_ClauseType>(clause)) != NA;         \
    }                                                                                \
    inline void _EnterClause##clause() {                                             \
        _curr_clause_type = static_cast<_ClauseType>(_curr_clause_type | clause);    \
    }                                                                                \
    inline void _LeaveClause##clause() {                                             \
        _curr_clause_type = static_cast<_ClauseType>(_curr_clause_type & (~clause)); \
    }

    CLAUSE_HELPER_FUNC(MATCH);
    CLAUSE_HELPER_FUNC(RETURN);
    CLAUSE_HELPER_FUNC(WITH);
    CLAUSE_HELPER_FUNC(UNWIND);
    CLAUSE_HELPER_FUNC(WHERE);
    CLAUSE_HELPER_FUNC(ORDERBY);
    CLAUSE_HELPER_FUNC(CREATE);
    CLAUSE_HELPER_FUNC(DELETE);
    CLAUSE_HELPER_FUNC(SET);
    CLAUSE_HELPER_FUNC(REMOVE);
    CLAUSE_HELPER_FUNC(MERGE);
    CLAUSE_HELPER_FUNC(INQUERYCALL);
};

}  // namespace parser