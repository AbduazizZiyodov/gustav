import typing as t
from dataclasses import dataclass

from gustav.enums import TokenType

__all__ = ("Token",)


@dataclass(frozen=True, slots=True, eq=False)
class Token:
    """
    Thing that I spend my 2-3 days. One moment, I tested random program:

        for (var i = 0; i < 10; i = i + 1) {
            print "hell yeah " ++ i;
        }

    Interpreter crashed because it could not find variable called "i" ( to be more specific: in the given distance which was 0).
    It happened after introducing resolver & scopes in interpreter. I thought, maybe I did some stupid mistake in that part ?!

    Re-wrote, refactored - nothing.

    Until, I printed contents of local variables dict in terminal.
    To understand clearly, we need to see how tokens are consumed by our interpreter & how global/local variables are stored.

    In the given code above, we are declaring/defining "i", then accessing "i" for conditional, then incrementing it.

    So, in this line we are consuming more than one  "i" kind tokens, and their content - IDENTICAL (even line number).

    At that time, our locals dictionary must look like this (note: x,y arbitrary line numbers):

    {
        Variable(name=Token(type=<TokenType.IDENTIFIER: 'identifier'>, lexeme='i', literal=None, line=X)): 0,
        Variable(name=Token(type=<TokenType.IDENTIFIER: 'identifier'>, lexeme='i', literal=None, line=y)): 2,
        Variable(name=Token(type=<TokenType.IDENTIFIER: 'identifier'>, lexeme='i', literal=None, line=X)): 1,
        Assign(name=Token(type=<TokenType.IDENTIFIER: 'identifier'>, lexeme='i', literal=None, line=X), <binary expr omitted>): 1
    }

    Value of dictionary represents "depth", there is a lot more to talk. But, if you look at line 1 and 3 items on that dict,
    you can see here is a tricky part !!!

    They are identical(keys), even though, they should have different depth. That's what we are expecting, no matter what (line or other attrs are same),
    they should be treated non identical !!!

    Is that so in our case (during interpreter crash)?

    {
        Variable(name=Token(type=<TokenType.IDENTIFIER: 'identifier'>, lexeme='i', literal=None, line=X)): 1,
        Variable(name=Token(type=<TokenType.IDENTIFIER: 'identifier'>, lexeme='i', literal=None, line=y)): 2,
        Assign(name=Token(type=<TokenType.IDENTIFIER: 'identifier'>, lexeme='i', literal=None, line=X), <binary expr omitted>): 1
    }

    Hell yeah, I was trying to access depth 0 "i" variable (during increment step i guess), and there is nothing. From logs, I see that
    depth 0,1,2,1 are being resolved. But where did go 0 ?! Locals dictionary inside interpreter was smoking :(

    As I said, tokens with the same content to be treated as equal. Thus, entries in locals dictionary overwrote each other even though they referred
    to different nodes in the ast.

    That's how you can shoot yourself in foot with your fancy dataclasses.

    @dataclass(frozen=True, slots=True)
    class Token:
        ...

    Nothing suspicious, but I've checked docs (you also should). I got nothing wrong with hashability (frozen was set to True). slots=True, because
    there might be a lot of token, which we can save some memory ( I understand, let's skip it ).

    But, one default param for dataclasses that related to topic, caused the this disaster:

    eq=True

    Yes. This stuff treats tokens that appear at the same line as identical (based on their content, not identity), I've set this as False.

    So that frozen is true, dataclass implements __hash__,
    and __eq__ is will not be generated. So it uses object.__eq__ => identity-based (x is y) - which is what I want.

    So far, I haven't encountered with any issues.

    * Read: https://docs.python.org/3/library/dataclasses.html#module-contents ( I did, btw )
    """

    type: TokenType
    lexeme: str
    literal: t.Any
    line: int
