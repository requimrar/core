// type.cpp
// Copyright (c) 2014 - 2017, zhiayang@gmail.com
// Licensed under the Apache License Version 2.0.

#include "pts.h"
#include "errors.h"
#include "ir/type.h"
#include "typecheck.h"

namespace sst
{
	fir::Type* TypecheckState::convertParserTypeToFIR(pts::Type* pt)
	{
		#define convert(...)	(this->convertParserTypeToFIR)(__VA_ARGS__)

		if(pt->isNamedType())
		{
			auto builtin = fir::Type::fromBuiltin(pt->toNamedType()->str());
			if(builtin)
			{
				return builtin;
			}
			else
			{
				error(this->loc(), "shouldn't be here yet");
			}
		}
		else if(pt->isPointerType())
		{
			return convert(pt->toPointerType()->base)->getPointerTo();
		}
		else if(pt->isTupleType())
		{
			std::vector<fir::Type*> ts;
			for(auto t : pt->toTupleType()->types)
				ts.push_back(convert(t));

			return fir::TupleType::get(ts);
		}
		else if(pt->isArraySliceType())
		{
			return fir::ArraySliceType::get(convert(pt->toArraySliceType()->base));
		}
		else if(pt->isDynamicArrayType())
		{
			return fir::DynamicArrayType::get(convert(pt->toDynamicArrayType()->base));
		}
		else if(pt->isFixedArrayType())
		{
			return fir::ArrayType::get(convert(pt->toFixedArrayType()->base), pt->toFixedArrayType()->size);
		}
		else if(pt->isFunctionType())
		{
			std::vector<fir::Type*> ps;
			for(auto p : pt->toFunctionType()->argTypes)
				ps.push_back(convert(p));

			auto ret = convert(pt->toFunctionType()->returnType);
			return fir::FunctionType::get(ps, ret);
		}
		else
		{
			error(this->loc(), "Invalid pts::Type found");
		}
	}
}

















