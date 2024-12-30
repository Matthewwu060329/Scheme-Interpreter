#include "Def.hpp"
#include "value.hpp"
#include "expr.hpp"
#include "RE.hpp"
#include "syntax.hpp"
#include <cstring>
#include <vector>
#include <map>

extern std :: map<std :: string, ExprType> primitives;
extern std :: map<std :: string, ExprType> reserved_words;

Value Let::eval(Assoc &env) {
    if(body.get() == nullptr) throw RuntimeError("Wrong format of 'Let'");

    Assoc e = env;
    for(int i = 0; i < bind.size(); i++) {
        std::string varName = bind[i].first;
        Expr expr = bind[i].second;
        Value value = expr->eval(e);
        env = Assoc(new AssocList(varName, value, env));
    }

    return body->eval(env);
} // let expression

Value Lambda::eval(Assoc &env) {
    return ClosureV(x,e,env);
} // lambda expression

Value Apply::eval(Assoc &e) {
    Value v0 = rator->eval(e);
    Closure* clo = dynamic_cast<Closure*>(v0.get());
    if(clo != nullptr) {
        if(clo->parameters.size() != rand.size())
            throw RuntimeError(std::string("Wrong number of rand " + std::to_string(clo->parameters.size()) + std::to_string(rand.size()) + " parameters"));
        Assoc env = clo->env;
        for(int i = 0; i < clo->parameters.size(); i++) {
            env = Assoc(new AssocList(clo->parameters[i],rand[i]->eval(e),env));
        }
        return clo->e->eval(env);
    }
    throw RuntimeError(std::string("rator->eval(e) is not a closure"));
} // for function calling

Value Letrec::eval(Assoc &env) {
    if(body.get() == nullptr) throw RuntimeError("Wrong format of 'Letrec'");

    Assoc env1 = env;
    for(int i = 0; i < bind.size(); i++) {
        std::string varName = bind[i].first;
        env1 = Assoc(new AssocList(varName, Value(nullptr), env1));
    }

    Assoc env2 = env1;
    for(int i = 0; i < bind.size(); i++) {
        std::string varName = bind[i].first;
        Expr expr = bind[i].second;
        Value value = expr->eval(env1);
        env2 = Assoc(new AssocList(varName, value, env2));
    }

    return body->eval(env2);
} // letrec expression

Value Var::eval(Assoc &e) {
    int dig = x[0] - '0';
    if((dig >= 0 && dig <= 9) || x[0] == '.' || x[0] == '@') throw RuntimeError(std::string("Illegal Identifier"));
    for(int i = 0; i < x.size(); i++) if(x[i] == '#') throw RuntimeError(std::string("Illegal Identifier"));

    Value v = find(x,e);
    if(v.get() != nullptr) return v;

    auto iter1 = primitives.find(x);
    if(iter1 != primitives.end()) {
        ExprType etype = iter1->second;
        std::vector<std::string> xs;
        Expr expr = nullptr;
        if(etype == E_EQQ || etype == E_CONS || etype == E_PLUS || etype == E_MINUS || etype == E_MUL ||
            etype == E_LT || etype == E_LE || etype == E_GT || etype == E_GE || etype == E_EQ) {
            xs.push_back("var1");
            xs.push_back("var2");

            if(etype == E_EQQ) expr = Expr(new IsEq(new Var("var1"), new Var("var2")));
            else if(etype == E_CONS) expr = Expr(new Cons(new Var("var1"), new Var("var2")));

            else if(etype == E_PLUS) expr = Expr(new Plus(new Var("var1"), new Var("var2")));
            else if(etype == E_MINUS) expr = Expr(new Minus(new Var("var1"), new Var("var2")));
            else if(etype == E_MUL) expr = Expr(new Mult(new Var("var1"), new Var("var2")));

            else if(etype == E_LT) expr = Expr (new Less(new Var("var1"), new Var("var2")));
            else if(etype == E_LE) expr = Expr(new LessEq(new Var("var1"), new Var("var2")));
            else if(etype == E_GT) expr = Expr(new Greater(new Var("var1"), new Var("var2")));
            else if(etype == E_GE) expr = Expr(new GreaterEq(new Var("var1"), new Var("var2")));
            else if(etype == E_EQ) expr = Expr (new Equal(new Var("var1"), new Var("var2")));
        }
        else if(etype == E_BOOLQ || etype == E_INTQ || etype == E_NULLQ || etype == E_PAIRQ ||
            etype == E_PROCQ || etype == E_SYMBOLQ || etype == E_NOT || etype == E_CAR || etype == E_CDR) {
            xs.push_back("var");

            if(etype == E_BOOLQ) {expr = Expr(new IsBoolean(new Var("var")));}
            else if(etype == E_INTQ) expr = Expr(new IsFixnum(new Var("var")));
            else if(etype == E_NULLQ) expr = Expr(new IsNull(new Var("var")));
            else if(etype == E_PAIRQ) expr = Expr(new IsPair(new Var("var")));
            else if(etype == E_PROCQ) expr = Expr(new IsProcedure(new Var("var")));
            else if(etype == E_SYMBOLQ) expr = Expr(new IsSymbol(new Var("var")));

            else if(etype == E_NOT) expr = Expr(new Not(new Var("var")));
            else if(etype == E_CAR) expr = Expr(new Car(new Var("var")));
            else if(etype == E_CDR) expr = Expr(new Cdr(new Var("var")));
        }
        else if(etype == E_VOID || etype == E_EXIT) {
            if(etype == E_VOID) expr = Expr(new MakeVoid());
            else if(etype == E_EXIT) expr = Expr(new Exit());
        }
        return ClosureV(xs, expr, e);
    }

    throw RuntimeError(std::string("Undefined identifier"));
} // evaluation of variable

Value Fixnum::eval(Assoc &e) {return Value(new Integer(n));} // evaluation of a fixnum

Value If::eval(Assoc &e) {
    if(cond.get() == nullptr) throw RuntimeError(std::string("Wrong number of variables"));
    Value val1 = cond->eval(e);
    Boolean* boo = dynamic_cast<Boolean*>(val1.get());
    if(boo != nullptr && boo->b == false) return alter->eval(e);
    return conseq->eval(e);
} // if expression

Value True::eval(Assoc &e) {return Value(new Boolean(true));} // evaluation of #t

Value False::eval(Assoc &e) {return Value(new Boolean(false));} // evaluation of #f

Value Begin::eval(Assoc &e) {
    if(es.size() == 0) throw RuntimeError(std::string("Wrong number of variables"));
    Value val = es[0]->eval(e);
    for(int i = 1; i < es.size(); i++) val = es[i]->eval(e);
    return val;
} // begin expression

Value Syntax::preeval(Assoc &e) {
    Number* num = dynamic_cast<Number*>(ptr.get());
    if(num != nullptr) return IntegerV(num->n);

    Identifier* id = dynamic_cast<Identifier*>(ptr.get());
    if(id != nullptr) return SymbolV(id->s);

    TrueSyntax* tru = dynamic_cast<TrueSyntax*>(ptr.get());
    if(tru != nullptr) BooleanV(true);

    FalseSyntax* fal = dynamic_cast<FalseSyntax*>(ptr.get());
    if(fal != nullptr) BooleanV(false);

    List* list = dynamic_cast<List*>(ptr.get());
    if(list != nullptr) {
        int sum = list->stxs.size();
        if(sum == 1) {
            Identifier* id0 = dynamic_cast<Identifier*>(list->stxs[0].get());
            if(id0 != nullptr && id0->s == ".") throw RuntimeError(std::string("Wrong format of identifier"));
            return PairV(list->stxs[0].preeval(e), NullV());
        }
        else if(sum == 2) {
            Identifier* id1 = dynamic_cast<Identifier*>(list->stxs[0].get());
            Identifier* id2 = dynamic_cast<Identifier*>(list->stxs[1].get());
            if(id1 != nullptr && id1->s == ".") throw RuntimeError(std::string("Wrong format of identifier"));
            if(id2 != nullptr && id2->s == ".") throw RuntimeError(std::string("Wrong format of identifier"));
            return PairV(list->stxs[0].preeval(e), list->stxs[1].preeval(e));
        }
        else if (sum >= 3) {
            int num = 0;
            for(int i = 0; i < list->stxs.size(); i++) {
                Identifier* id = dynamic_cast<Identifier*>(list->stxs[i].get());
                if(id != nullptr && id->s == ".") num++;
            }
            if(num >= 2) throw RuntimeError(std::string("Wrong format of identifier"));
            if(num == 1) {
                Identifier* ide = dynamic_cast<Identifier*>(list->stxs[list->stxs.size() - 2].get());
                if(!(ide != nullptr && ide->s == ".")) throw RuntimeError(std::string("Wrong format of identifier"));
            }
            Value pair = PairV(list->stxs[sum - 1].preeval(e),NullV());
            for(int i = sum - 2; i >= 0; i--) {
                pair = PairV(list->stxs[i].preeval(e), pair);
            }
            return pair;
        }
        return NullV();
    }
    if(ptr.get() == nullptr) return NullV();
    throw RuntimeError(std::string("Data type error"));
}

Value Quote::eval(Assoc &e) {
    if(s.get() == nullptr) throw RuntimeError(std::string("Wrong number of variables"));
    return s.preeval(e);
} // quote expression

Value MakeVoid::eval(Assoc &e) {return Value(new Void());} // (void)

Value Exit::eval(Assoc &e) {return Value(new Terminate());} // (exit)

Value Binary::eval(Assoc &e) {return this->evalRator(rand1->eval(e), rand2->eval(e));} // evaluation of two-operators primitive

Value Unary::eval(Assoc &e) {return this->evalRator(rand->eval(e));} // evaluation of single-operator primitive

Value Mult::evalRator(const Value &rand1, const Value &rand2) {
    if(rand1->v_type != V_INT || rand2->v_type != V_INT)
        throw RuntimeError(std::string("Wrong data type"));
    return Value(new Integer(dynamic_cast<Integer*>(rand1.get())->n *
        dynamic_cast<Integer*>(rand2.get())->n));
} // *

Value Plus::evalRator(const Value &rand1, const Value &rand2) {
    if(rand1->v_type != V_INT || rand2->v_type != V_INT)
        throw RuntimeError(std::string("Wrong data type"));
    return Value(new Integer(dynamic_cast<Integer*>(rand1.get())->n +
        dynamic_cast<Integer*>(rand2.get())->n));
} // +

Value Minus::evalRator(const Value &rand1, const Value &rand2) {
    if(rand1->v_type != V_INT || rand2->v_type != V_INT)
        throw RuntimeError(std::string("Wrong data type"));
    return Value(new Integer(dynamic_cast<Integer*>(rand1.get())->n -
        dynamic_cast<Integer*>(rand2.get())->n));
} // -

Value Less::evalRator(const Value &rand1, const Value &rand2) {
    if(rand1->v_type != V_INT || rand2->v_type != V_INT)
        throw RuntimeError(std::string("Wrong data type"));
    return Value(new Boolean(dynamic_cast<Integer*>(rand1.get())->n <
        dynamic_cast<Integer*>(rand2.get())->n));
} // <

Value LessEq::evalRator(const Value &rand1, const Value &rand2) {
    if(rand1->v_type != V_INT || rand2->v_type != V_INT)
        throw RuntimeError(std::string("Wrong data type"));
    return Value(new Boolean(dynamic_cast<Integer*>(rand1.get())->n <=
        dynamic_cast<Integer*>(rand2.get())->n));
} // <=

Value Equal::evalRator(const Value &rand1, const Value &rand2) {
    if(rand1->v_type != V_INT || rand2->v_type != V_INT)
        throw RuntimeError(std::string("Wrong data type"));
    return Value(new Boolean(dynamic_cast<Integer*>(rand1.get())->n ==
        dynamic_cast<Integer*>(rand2.get())->n));
} // =

Value GreaterEq::evalRator(const Value &rand1, const Value &rand2) {
    if(rand1->v_type != V_INT || rand2->v_type != V_INT)
        throw RuntimeError(std::string("Wrong data type"));
    return Value(new Boolean(dynamic_cast<Integer*>(rand1.get())->n >=
        dynamic_cast<Integer*>(rand2.get())->n));
} // >=

Value Greater::evalRator(const Value &rand1, const Value &rand2) {
    if(rand1->v_type != V_INT || rand2->v_type != V_INT)
        throw RuntimeError(std::string("Wrong data type"));
    return Value(new Boolean(dynamic_cast<Integer*>(rand1.get())->n >
        dynamic_cast<Integer*>(rand2.get())->n));
} // >

Value IsEq::evalRator(const Value &rand1, const Value &rand2) {
    Integer* num1 = dynamic_cast<Integer*>(rand1.get());
    Integer* num2 = dynamic_cast<Integer*>(rand2.get());
    if(num1 != nullptr && num2 != nullptr) {
        if(num1->n == num2->n) return Value(new Boolean(true));
        return Value(new Boolean(false));
    }

    Boolean* boo1 = dynamic_cast<Boolean*>(rand1.get());
    Boolean* boo2 = dynamic_cast<Boolean*>(rand2.get());
    if(boo1 != nullptr && boo2 != nullptr) {
        if(boo1->b == boo2->b) return Value(new Boolean(true));
        return Value(new Boolean(false));
    }

    Symbol* sym1 = dynamic_cast<Symbol*>(rand1.get());
    Symbol* sym2 = dynamic_cast<Symbol*>(rand2.get());
    if(sym1 != nullptr && sym2 != nullptr) {
        if(sym1->s == sym2->s) return Value(new Boolean(true));
        return Value(new Boolean(false));
    }

    Null* null1 = dynamic_cast<Null*>(rand1.get());
    Null* null2 = dynamic_cast<Null*>(rand2.get());
    if(null1 != nullptr && null2 != nullptr) {return Value(new Boolean(true));}

    Void* void1 = dynamic_cast<Void*>(rand1.get());
    Void* void2 = dynamic_cast<Void*>(rand2.get());
    if(void1 != nullptr && void2 != nullptr) {return Value(new Boolean(true));}

    if(rand1.get() == rand2.get()) return Value(new Boolean(true));
    return Value(new Boolean(false));
} // eq?

Value Cons::evalRator(const Value &rand1, const Value &rand2) {return Value(new Pair(rand1, rand2));} // cons

Value IsBoolean::evalRator(const Value &rand) {
    Boolean* boo = dynamic_cast<Boolean*>(rand.get());
    if(boo != nullptr) return Value(new Boolean(true));
    return Value(new Boolean(false));
} // boolean?

Value IsFixnum::evalRator(const Value &rand) {
    Integer* num = dynamic_cast<Integer*>(rand.get());
    if(num != nullptr) return Value(new Boolean(true));
    return Value(new Boolean(false));
} // fixnum?

Value IsSymbol::evalRator(const Value &rand) {
    Symbol* sym = dynamic_cast<Symbol*>(rand.get());
    if(sym != nullptr) return Value(new Boolean(true));
    return Value(new Boolean(false));
} // symbol?

Value IsNull::evalRator(const Value &rand) {
    Null* null = dynamic_cast<Null*>(rand.get());
    if(null != nullptr) return Value(new Boolean(true));
    return Value(new Boolean(false));
} // null?

Value IsPair::evalRator(const Value &rand) {
    Pair* pair = dynamic_cast<Pair*>(rand.get());
    if(pair != nullptr) return Value(new Boolean(true));
    return Value(new Boolean(false));
} // pair?

Value IsProcedure::evalRator(const Value &rand) {
    Closure* clo = dynamic_cast<Closure*>(rand.get());
    if(clo != nullptr) return Value(new Boolean(true));
    return Value(new Boolean(false));
} // procedure?

Value Not::evalRator(const Value &rand) {
    Boolean* boo = dynamic_cast<Boolean*>(rand.get());
    if(boo !=nullptr || boo->b == false) return Value(new Boolean(true));
    return Value(new Boolean(false));
} // not

Value Car::evalRator(const Value &rand) {
    Pair* pair = dynamic_cast<Pair*>(rand.get());
    if(pair == nullptr) throw RuntimeError(std::string("Inappropriate pair type error"));
    return pair->car;
} // car

Value Cdr::evalRator(const Value &rand) {
    Pair* pair = dynamic_cast<Pair*>(rand.get());
    if(pair == nullptr) throw RuntimeError(std::string("Inappropriate pair type error"));
    Value subpair = pair->cdr;
    Pair* ispair = dynamic_cast<Pair*>(subpair.get());
    if(ispair != nullptr) {
        Symbol* ide = dynamic_cast<Symbol*>(ispair->car.get());
        if(ide != nullptr && ide->s == ".") {
            Pair* final_pair = dynamic_cast<Pair*>(ispair->cdr.get());
            if(final_pair == nullptr) return ispair->cdr;
            return final_pair->car;
        }
    }
    return pair->cdr;
} // cdr
