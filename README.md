# General info

In the course Compilers - CSM14204 in university of Helsinki, the assignment was to implement an interpreter for the predefined programming language called MiniPL.
MiniPL is a language designed for pedagogical porposes and the objective in the course was to learn the structure and implementation techniques of compilers and interpreters via the implementation of the MiniPL language.
The documentation describes the architecture of the implementation, error handling and testing. The original assignment including the MiniPL grammar and its associated semantic rules can be found in the end of the documentation. Since the implementation were supposed to be done using an LL(1) parser, the original language definition are rewritten to be LL(1) compatible. Additionally of the new grammar specification, the lexical elements are defined using regular expressions.