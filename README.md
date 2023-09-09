# Classroom Object Oriented Language (COOL) Compiler

## Table of Contents
- [About the Project](#about-the-project)
- [Built With](#built-with)
- [Getting Started](#getting-started)
- [Prerequisites](#prerequisites)
- [Usage](#usage)
  - [A Stack Machine written in COOL](#a-stack-machine-written-in-cool)
  - [Phase One: Lexer with Flex](#phase-one-lexer-with-flex)
  - [Phase Two: Parser with Bison](#phase-two-parser-with-bison)
  - [Phase Three: Semantical Analysis](#phase-three-semantical-analysis)
- [Contact](#contact)

## About The Project
Welcome to the Classroom Object Oriented Language (COOL) Compiler project! This project is part of a compiler course, aiming to design a compiler for COOL, a unique programming language. Here's an overview of what this project entails:

- **A Stack Machine in COOL**: To kick things off, we implemented a stack machine using the COOL language. This machine operates with a single stack for storage, allowing us to execute basic commands and perform computations.

- **Phase One: Lexer with Flex**: In this phase, we developed a lexical analyzer (or scanner) using Flex. Our scanner identifies tokens in COOL programs, helping us catch some syntax errors. 

- **Phase Two: Parser with Bison**: Phase two introduces a COOL syntax analyzer (parser) built with Bison. This parser defines the grammar for the language and captures additional syntax errors in COOL programs.

- **Phase Three: Semantical Analysis**: In the final phase, we implemented the static semantics of COOL. This includes building an inheritance graph, checking its well-formedness, and performing type checking on expressions. Our semantic analyzer detects and reports various semantic errors in COOL programs.

This project is inspired by a well-known assignment from Stanford University. We utilized their base code as a starting point and have detailed our contributions in this README. We've also tested our code with the Stanford grader, achieving near-passing results on all tests.


## Built With
- C++
- Flex
- Bison

## Getting Started

### A Stack Machine written in COOL
We've created a primitive stack machine in COOL. This machine operates with a single stack for storage and includes basic commands for programming.

### Phase One: Lexer with Flex
In this phase, we've designed a lexical analyzer (scanner) using Flex. It identifies tokens in COOL programs, helping catch certain syntax errors.

### Phase Two: Parser with Bison
Phase two involves building a COOL parser using Bison. This parser defines the language's grammar and identifies additional syntax errors in COOL code. The output is an abstract syntax tree (AST).

### Phase Three: Semantical Analysis
In the final phase, we've implemented COOL's static semantics. The semantic analyzer checks that a program adheres to COOL specifications, rejecting erroneous programs and annotating the AST with types. It also detects various semantic errors possible in COOL.

