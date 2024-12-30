#ifndef PARSER 
#define PARSER

// parser of myscheme 

#include "RE.hpp"
#include "Def.hpp"
#include "syntax.hpp"
#include "expr.hpp"
#include "value.hpp"
#include <map>
#include <cstring>
#include <iostream>
#define mp make_pair
using std :: string;
using std :: vector;
using std :: pair;

extern std :: map<std :: string, ExprType> primitives;
extern std :: map<std :: string, ExprType> reserved_words;

Expr Syntax :: parse(Assoc &env) {return ptr->parse(env);}

Expr Number :: parse(Assoc &env) {return Expr(new Fixnum(n));}

Expr Identifier :: parse(Assoc &env) {return Expr(new Var(s));}

Expr TrueSyntax :: parse(Assoc &env) {return Expr(new True());}

Expr FalseSyntax :: parse(Assoc &env) {return Expr(new False());}

Expr List :: parse(Assoc &env) {
    if(stxs.empty()) return Expr(new MakeVoid());

    Identifier* var = dynamic_cast<Identifier*>(stxs[0].get());
    if(var == nullptr) {
        vector<Expr> rand;
        for(int i = 1; i < stxs.size(); i++) {
            rand.push_back(stxs[i].parse(env));
        }
        return Expr(new Apply(stxs[0].parse(env), rand));
    }
    else {
        string str = var->s;

        // auto iter1 = primitives.find(str);
        // if(iter1 != primitives.end()) {
        //     vector<Expr> rand;
        //     for(int i = 1; i < stxs.size(); i++) rand.push_back(stxs[i].parse(env));
        //     return Expr(new Apply(stxs[0].parse(env), rand));
        // }

        auto iter2 = reserved_words.find(str);
        if(iter2 != reserved_words.end()) {
            ExprType expr = iter2->second;
            if(expr == E_LAMBDA) {
                if(stxs.size() != 3) throw RuntimeError(string("Wrong number of arguments"));
                List* list1 = dynamic_cast<List*>(stxs[1].get());
                if(list1 == nullptr) throw RuntimeError(string("Data type error"));
                vector<std::string> xs;
                Expr expr = nullptr;
                for(int i = 0; i < list1->stxs.size(); i++) {
                    Identifier* var = dynamic_cast<Identifier*>(list1->stxs[i].get());
                    if(var == nullptr) throw RuntimeError(string("Data type error"));
                    xs.push_back(var->s);
                }
                return Expr(new Lambda(xs,stxs[2].parse(env)));
            }
            if(expr == E_IF) {
                if(stxs.size() != 4) return Expr(new If(Expr(nullptr),Expr(nullptr),Expr(nullptr)));
                return Expr(new If(stxs[1].parse(env), stxs[2].parse(env), stxs[3].parse(env)));
            }
            if(expr == E_QUOTE) {
                if(stxs.size() != 2) return Expr(new Quote(Syntax(nullptr)));
                return Expr(new Quote(stxs[1]));
            }
            if(expr == E_BEGIN) {
                vector<Expr> es;
                for(int i = 1; i < stxs.size(); i++) es.push_back(stxs[i].parse(env));
                return Expr(new Begin(es));
            }
            if(expr == E_LET) {
                vector<pair<string,Expr>> bind;
                if(stxs.size() != 3) return Expr(new Let(bind,Expr(nullptr)));

                List* list1 = dynamic_cast<List*>(stxs[1].get());
                MakeVoid* void1 = dynamic_cast<MakeVoid*>(stxs[1].get());
                if(list1 == nullptr && void1 == nullptr) return Expr(new Let(bind,Expr(nullptr)));
                if(list1 != nullptr) {
                    for(int i = 0; i < list1->stxs.size(); i++) {
                        List* list0 = dynamic_cast<List*>(list1->stxs[i].get());
                        if(list0 == nullptr) return Expr(new Let(bind,Expr(nullptr)));
                        if(list0->stxs.size() != 2) return Expr(new Let(bind,Expr(nullptr)));

                        Identifier* unbind = dynamic_cast<Identifier*>(list0->stxs[0].get());
                        if(unbind == nullptr) return Expr(new Let(bind,Expr(nullptr)));
                        bind.push_back(make_pair(unbind->s, list0->stxs[1].parse(env)));
                    }
                }

                return Expr(new Let(bind,stxs[2].parse(env)));
            }
            if(expr == E_LETREC) {
                vector<pair<string,Expr>> bind;
                if(stxs.size() != 3) return Expr(new Letrec(bind,Expr(nullptr)));

                List* list1 = dynamic_cast<List*>(stxs[1].get());
                MakeVoid* void1 = dynamic_cast<MakeVoid*>(stxs[1].get());
                if(list1 == nullptr && void1 == nullptr) return Expr(new Letrec(bind,Expr(nullptr)));
                if(list1 != nullptr) {
                    for(int i = 0; i < list1->stxs.size(); i++) {
                        List* list0 = dynamic_cast<List*>(list1->stxs[i].get());
                        if(list0 == nullptr) return Expr(new Letrec(bind,Expr(nullptr)));
                        if(list0->stxs.size() != 2) return Expr(new Letrec(bind,Expr(nullptr)));

                        Identifier* unbind = dynamic_cast<Identifier*>(list0->stxs[0].get());
                        if(unbind == nullptr) return Expr(new Letrec(bind,Expr(nullptr)));
                        bind.push_back(make_pair(unbind->s, list0->stxs[1].parse(env)));
                    }
                }

                return Expr(new Letrec(bind,stxs[2].parse(env)));
            }
        }
        vector<Expr> rand;
        for(int i = 1; i < stxs.size(); i++) rand.push_back(stxs[i].parse(env));
        return Expr(new Apply(stxs[0].parse(env), rand));
    }
}

#endif
