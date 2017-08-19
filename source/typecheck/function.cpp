// function.cpp
// Copyright (c) 2014 - 2017, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "ast.h"
#include "errors.h"
#include "typecheck.h"

#include "ir/type.h"

using TCS = sst::TypecheckState;

#define dcast(t, v)		dynamic_cast<t*>(v)


sst::Stmt* ast::FuncDefn::typecheck(TCS* fs, fir::Type* inferred)
{
	fs->pushLoc(this);
	defer(fs->popLoc());

	if(this->generics.size() > 0)
	{
		fs->stree->unresolvedGenericFunctions[this->name].push_back(this);
		return 0;
	}

	using Param = sst::FunctionDefn::Param;
	auto defn = new sst::FunctionDefn(this->loc);
	std::vector<Param> ps;

	for(auto t : this->args)
		ps.push_back(Param { .name = t.name, .type = fs->convertParserTypeToFIR(t.type) });

	auto retty = fs->convertParserTypeToFIR(this->returnType);

	// todo: check overloading/duplication

	defn->params = ps;
	defn->name = this->name;
	defn->returnType = retty;
	defn->privacy = this->privacy;

	defn->body = new sst::Block(this->body->loc);

	// do the body
	for(auto stmt : this->body->statements)
		defn->body->statements.push_back(stmt->typecheck(fs));

	for(auto stmt : this->body->deferredStatements)
		defn->body->deferred.push_back(stmt->typecheck(fs));

	return defn;
}

sst::Stmt* ast::ForeignFuncDefn::typecheck(TCS* fs, fir::Type* inferred)
{
	fs->pushLoc(this);
	defer(fs->popLoc());

	using Param = sst::ForeignFuncDefn::Param;
	auto defn = new sst::ForeignFuncDefn(this->loc);
	std::vector<Param> ps;

	for(auto t : this->args)
		ps.push_back(Param { .name = t.name, .type = fs->convertParserTypeToFIR(t.type) });

	auto retty = fs->convertParserTypeToFIR(this->returnType);

	defn->params = ps;
	defn->name = this->name;
	defn->returnType = retty;
	defn->privacy = this->privacy;
	defn->isVarArg = defn->isVarArg;


	// add the defn to the current thingy
	if(fs->stree->foreignFunctions.find(defn->name) != fs->stree->foreignFunctions.end())
	{
		exitless_error(this->loc, "Function '%s' already exists; foreign functions cannot be overloaded", this->name.c_str());
		info(fs->stree->foreignFunctions[this->name]->loc, "Previously declared here:");

		doTheExit();
	}

	fs->stree->foreignFunctions[this->name] = defn;
	return defn;
}















