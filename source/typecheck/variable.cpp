// variable.cpp
// Copyright (c) 2014 - 2017, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "pts.h"
#include "ast.h"
#include "errors.h"
#include "typecheck.h"

#include "ir/type.h"

using TCS = sst::TypecheckState;

#define dcast(t, v)		dynamic_cast<t*>(v)

sst::Expr* ast::Ident::typecheck(sst::TypecheckState* fs, fir::Type* infer)
{
	fs->pushLoc(this);
	defer(fs->popLoc());

	// hm.
	auto tree = fs->stree;
	while(tree)
	{
		auto vs = tree->definitions[this->name];
		if(vs.size() > 1)
		{
			if(infer == 0)
			{
				exitless_error(this, "Ambiguous reference to '%s'", this->name);
				for(auto v : vs)
					info(v, "Potential target here:");

				doTheExit();
			}


			// ok, attempt.
			// it's probably a function, anyway
			for(auto v : vs)
			{
				if(v->type == infer)
				{
					auto ret = new sst::VarRef(this->loc);
					ret->name = this->name;
					ret->type = v->type;
					ret->def = v;

					return ret;
				}
			}

			exitless_error(this, "No definition of '%s' matching type '%s'", this->name, infer->str());
			for(auto v : vs)
				info(v, "Potential target here, with type '%s':", v->type ? v->type->str() : "?");

			doTheExit();
		}
		else if(!vs.empty())
		{
			auto def = vs.front();
			iceAssert(def);
			{
				auto ret = new sst::VarRef(this->loc);
				ret->name = this->name;
				ret->type = def->type;
				ret->def = def;

				return ret;
			}
		}

		tree = tree->parent;
	}

	// ok, we haven't found anything
	error(this, "Reference to unknown variable '%s'", this->name);
}




sst::Stmt* ast::VarDefn::typecheck(sst::TypecheckState* fs, fir::Type* infer)
{
	fs->pushLoc(this);
	defer(fs->popLoc());

	// ok, then.
	auto defn = new sst::VarDefn(this->loc);
	defn->id = Identifier(this->name, IdKind::Name);
	defn->id.scope = fs->getCurrentScope();

	defn->immutable = this->immut;
	defn->privacy = this->privacy;

	defn->global = !fs->isInFunctionBody();

	if(this->type != pts::InferredType::get())
	{
		defn->type = fs->convertParserTypeToFIR(this->type);
	}
	else
	{
		if(!this->initialiser)
			error(this, "Initialiser is required for type inference");
	}

	fs->checkForShadowingOrConflictingDefinition(defn, "variable", [this](TCS* fs, sst::Defn* other) -> bool {
		if(auto v = dcast(sst::Defn, other))
			return v->id.name == this->name;

		else
			return true;
	});

	// check the defn
	if(this->initialiser)
	{
		defn->init = this->initialiser->typecheck(fs, defn->type);

		if(defn->type == 0)
		{
			auto t = defn->init->type;
			if(t->isConstantNumberType())
				t = fs->inferCorrectTypeForLiteral(defn->init);

			defn->type = t;
		}
	}

	fs->stree->definitions[this->name].push_back(defn);
	return defn;
}


















