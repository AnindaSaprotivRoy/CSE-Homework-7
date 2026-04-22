This project is a small matrix engine written in C.

The code reads matrices, stores them by name, and then
uses those matrices to evaluate expressions from script
files.

The heart of the project is hw7.c. It contains the
functions for matrix addition, multiplication, and
transpose. It also has the code that builds matrices
from text, converts infix expressions into postfix,
and evaluates those expressions step by step.

To keep track of matrices by letter name, the code
uses a basic binary search tree. That tree acts like a
symbol table. When a matrix is added, it is inserted
into the tree. When a formula needs a matrix, the code
looks it up in the tree by name.

The parser is designed around the assignment rules.
It accepts matrix definitions such as A = 2 3 [ ... ]
and formulas such as A + B * C or (A + B)' * D.
The infix-to-postfix step makes the evaluation easier,
because the code can process operators in the correct
order without getting tangled in parentheses.

Memory handling is a big part of the assignment, so
the implementation tries to be careful about malloc
and free. Matrices are stored in one contiguous block,
and the tree nodes are freed recursively when they are
no longer needed. Temporary matrices created while an
expression is being evaluated are also cleaned up.

The student tests in [student_tests.c] give a small
sample of coverage. They check addition, multiplication,
transpose, matrix creation, postfix conversion, BST
lookup, expression evaluation, and full script running.