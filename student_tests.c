#include "unit_tests.h"

#include <string.h>

TestSuite(student_tests, .timeout=TEST_TIMEOUT); 

Test(student_tests, add_rectangular_matrix) {
	matrix_sf *A = copy_matrix(2, 3, (int[]){1, -2, 3, 4, 5, -6});
	matrix_sf *B = copy_matrix(2, 3, (int[]){7, 8, -9, 10, -11, 12});
	matrix_sf *C = add_mats_sf(A, B);
	cr_assert_not_null(C);
	expect_matrices_equal(C, 2, 3, (int[]){8, 6, -6, 14, -6, 6});
	free(A);
	free(B);
	free(C);
}

Test(student_tests, mult_matrix_2x3_by_3x2) {
	matrix_sf *A = copy_matrix(2, 3, (int[]){1, 2, 3, 4, 5, 6});
	matrix_sf *B = copy_matrix(3, 2, (int[]){7, 8, 9, 10, 11, 12});
	matrix_sf *C = mult_mats_sf(A, B);
	cr_assert_not_null(C);
	expect_matrices_equal(C, 2, 2, (int[]){58, 64, 139, 154});
	free(A);
	free(B);
	free(C);
}

Test(student_tests, transpose_non_square_matrix) {
	matrix_sf *A = copy_matrix(2, 3, (int[]){2, 4, 6, 1, 3, 5});
	matrix_sf *AT = transpose_mat_sf(A);
	cr_assert_not_null(AT);
	expect_matrices_equal(AT, 3, 2, (int[]){2, 1, 4, 3, 6, 5});
	free(A);
	free(AT);
}

Test(student_tests, create_matrix_with_irregular_spacing) {
	matrix_sf *A = create_matrix_sf('A', " 2   3 [ 1  2 3 ; 4 5   6; ] ");
	cr_assert_not_null(A);
	cr_expect_eq(A->name, 'A');
	expect_matrices_equal(A, 2, 3, (int[]){1, 2, 3, 4, 5, 6});
	free(A);
}

Test(student_tests, infix_to_postfix_precedence) {
	char expr[] = "A+B*C";
	char *postfix = infix2postfix_sf(expr);
	cr_assert_not_null(postfix);
	cr_expect_str_eq(postfix, "ABC*+");
	free(postfix);
}

Test(student_tests, infix_to_postfix_with_transpose_and_parentheses) {
	char expr[] = "(A+B)'*C";
	char *postfix = infix2postfix_sf(expr);
	cr_assert_not_null(postfix);
	cr_expect_str_eq(postfix, "AB+'C*");
	free(postfix);
}

Test(student_tests, bst_insert_and_find_nodes) {
	bst_sf *root = NULL;
	matrix_sf *D = copy_matrix(1, 1, (int[]){4}); D->name = 'D';
	matrix_sf *B = copy_matrix(1, 1, (int[]){2}); B->name = 'B';
	matrix_sf *F = copy_matrix(1, 1, (int[]){6}); F->name = 'F';
	matrix_sf *A = copy_matrix(1, 1, (int[]){1}); A->name = 'A';
	matrix_sf *C = copy_matrix(1, 1, (int[]){3}); C->name = 'C';

	root = insert_bst_sf(D, root);
	root = insert_bst_sf(B, root);
	root = insert_bst_sf(F, root);
	root = insert_bst_sf(A, root);
	root = insert_bst_sf(C, root);

	cr_assert_not_null(find_bst_sf('A', root));
	cr_assert_not_null(find_bst_sf('C', root));
	cr_assert_not_null(find_bst_sf('F', root));
	cr_expect_null(find_bst_sf('Z', root));

	free_bst_sf(root);
}

Test(student_tests, evaluate_expression_with_precedence) {
	bst_sf *root = NULL;
	matrix_sf *A = copy_matrix(2, 2, (int[]){1, 2, 3, 4}); A->name = 'A';
	matrix_sf *B = copy_matrix(2, 2, (int[]){2, 0, 1, 2}); B->name = 'B';
	matrix_sf *C = copy_matrix(2, 2, (int[]){0, 1, 1, 0}); C->name = 'C';
	root = insert_bst_sf(A, root);
	root = insert_bst_sf(B, root);
	root = insert_bst_sf(C, root);

	char expr[] = "A+B*C";
	matrix_sf *R = evaluate_expr_sf('R', expr, root);
	cr_assert_not_null(R);
	cr_expect_eq(R->name, 'R');
	expect_matrices_equal(R, 2, 2, (int[]){1, 4, 5, 5});

	free(R);
	free_bst_sf(root);
}

Test(student_tests, evaluate_expression_with_transpose) {
	bst_sf *root = NULL;
	matrix_sf *A = copy_matrix(2, 2, (int[]){1, 3, 2, 4}); A->name = 'A';
	matrix_sf *B = copy_matrix(2, 2, (int[]){5, 6, 7, 8}); B->name = 'B';
	root = insert_bst_sf(A, root);
	root = insert_bst_sf(B, root);

	char expr[] = "A'+B";
	matrix_sf *R = evaluate_expr_sf('R', expr, root);
	cr_assert_not_null(R);
	expect_matrices_equal(R, 2, 2, (int[]){6, 8, 10, 12});

	free(R);
	free_bst_sf(root);
}

Test(student_tests, execute_script_end_to_end) {
	char template[] = "/tmp/hw7_script_XXXXXX";
	int fd = mkstemp(template);
	cr_assert_neq(fd, -1);

	const char *script =
		"A = 2 2 [1 2; 3 4;]\n"
		"B = 2 2 [5 6; 7 8;]\n"
		"C = (A + B') * A\n";
	ssize_t bytes_written = write(fd, script, strlen(script));
	cr_assert_eq(bytes_written, (ssize_t)strlen(script));
	close(fd);

	matrix_sf *R = execute_script_sf(template);
	cr_assert_not_null(R);
	cr_expect_eq(R->name, 'C');
	expect_matrices_equal(R, 2, 2, (int[]){33, 48, 45, 66});

	free(R);
	unlink(template);
}
