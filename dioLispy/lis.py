Symbol = str


class Env(dict):
    def __init__(self, parms=(), args=(), outer=None):
        self.update(zip(parms, args))
        self.outer = outer

    def find(self, var):
        return self if var in self else self.outer.find(var)


def add_globals(env):
    import math, operator as op

    env.update(vars(math))
    env.update(
        {
            # arithmetic operators
            "+": op.add,
            "-": op.sub,
            "*": op.mul,
            "/": op.floordiv,
            "mod": op.mod,
            "not": op.not_,
            # boolean operators
            ">": op.gt,
            "<": op.lt,
            ">=": op.ge,
            "<=": op.le,
            "=": op.eq,
            "equal?": op.eq,
            "eq?": op.is_,
            "zero?": lambda x: x == 0,
            "even?": lambda x: x % 2 == 0,
            "odd?": lambda x: x % 2 == 1,
            # list operators
            "length": len,
            "cons": lambda x, y: [x] + y,
            "car": lambda x: x[0],
            "cdr": lambda x: x[1:],
            "append": op.add,
            "list": lambda *x: list(x),
            "list?": lambda x: isa(x, list),
            "null?": lambda x: x == [],
            "symbol?": lambda x: isa(x, Symbol),
            # ordinary procedures
            "abs": lambda x: x if x >= 0 else - x,
            "sqrt": lambda x: math.sqrt(x),
            "inc": lambda x: x + 1,
            "dec": lambda x: x - 1,
        }
    )
    return env


global_env = add_globals(Env())

isa = isinstance


def eval(x, env=global_env):
    if isa(x, Symbol):
        return env.find(x)[x]
    elif not isa(x, list):
        return x
    elif x[0] == "quote":
        (_, exp) = x
        return exp
    elif x[0] == "if":
        (_, test, conseq, alt) = x
        return eval((conseq if eval(test, env) else alt), env)
    elif x[0] == "set!":
        (_, var, exp) = x
        env.find(var)[var] = eval(exp, env)
    elif x[0] == "define":  # (define var exp)
        (_, var, exp) = x
        env[var] = eval(exp, env)
    elif x[0] == "lambda":  # (lambda (var*) exp)
        (_, vars, exp) = x
        return lambda *args: eval(exp, Env(vars, args, env))
    elif x[0] == "begin":  # (begin exp*)
        for exp in x[1:]:
            val = eval(exp, env)
        return val
    else:  # (proc exp*)
        exps = [eval(exp, env) for exp in x]
        proc = exps.pop(0)
        return proc(*exps)


def read(s):
    return read_from(tokenize(s))


parse = read


def tokenize(s):
    return s.replace("(", " ( ").replace(")", " ) ").split()


def read_from(tokens):
    if len(tokens) == 0:
        raise SyntaxError("unexpected EOF while reading")
    token = tokens.pop(0)
    if "(" == token:
        L = []
        while tokens[0] != ")":
            L.append(read_from(tokens))
        tokens.pop(0)  # pop off ')'
        return L
    elif ")" == token:
        raise SyntaxError("unexpected )")
    else:
        return atom(token)


def atom(token):
    try:
        return int(token)
    except ValueError:
        try:
            return float(token)
        except ValueError:
            return Symbol(token)


def to_string(exp):
    return "(" + " ".join(map(to_string, exp)) + ")" if isa(exp, list) else str(exp)


def repl(prompt="lispy> "):
    while True:
        val = eval(parse(input(prompt)))
        if val is not None:
            print(to_string(val))

if __name__ == "__main__":
    repl()