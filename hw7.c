#include "hw7.h"

static matrix_sf *allocate_matrix(unsigned int num_rows, unsigned int num_cols) {
    // Allocate one block for struct metadata + matrix values.
    matrix_sf *mat = malloc(sizeof(matrix_sf) + num_rows * num_cols * sizeof(int));
    if (!mat) {
        return NULL;
    }
    mat->name = '?';
    mat->num_rows = num_rows;
    mat->num_cols = num_cols;
    return mat;
}

static char *trim_whitespace(char *str) {
    // Skip leading spaces.
    while (*str != '\0' && isspace((unsigned char)*str)) {
        str++;
    }
    if (*str == '\0') {
        return str;
    }
    // Remove trailing spaces in place.
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
    return str;
}

static int matrix_operand_precedence(char op) {
    // Multiplication has higher precedence than addition.
    switch (op) {
        case '+':
            return 1;
        case '*':
            return 2;
        default:
            return 0;
    }
}

static int is_matrix_literal_start(char ch) {
    return isdigit((unsigned char)ch) || ch == '[' || ch == '-';
}

static char *next_token(char **cursor) {
    // Move cursor to the next non-space token.
    while (**cursor != '\0' && isspace((unsigned char)**cursor)) {
        (*cursor)++;
    }
    return *cursor;
}

typedef struct {
    matrix_sf *mat;
    int owned;
} matrix_stack_item_sf;

static void free_matrix_stack(matrix_stack_item_sf *stack) {
    free(stack);
}

bst_sf* insert_bst_sf(matrix_sf *mat, bst_sf *root) {
    if (mat == NULL) {
        return root;
    }
    if (root == NULL) {
        // First insert creates the node.
        bst_sf *node = malloc(sizeof(bst_sf));
        if (!node) {
            return NULL;
        }
        node->mat = mat;
        node->left_child = NULL;
        node->right_child = NULL;
        return node;
    }
    if (mat->name < root->mat->name) {
        // Smaller names go left.
        root->left_child = insert_bst_sf(mat, root->left_child);
    } else {
        // Equal/larger names go right.
        root->right_child = insert_bst_sf(mat, root->right_child);
    }
    return root;
}

matrix_sf* find_bst_sf(char name, bst_sf *root) {
    if (root == NULL) {
        return NULL;
    }
    if (name == root->mat->name) {
        return root->mat;
    }
    if (name < root->mat->name) {
        return find_bst_sf(name, root->left_child);
    }
    return find_bst_sf(name, root->right_child);
}

void free_bst_sf(bst_sf *root) {
    if (root == NULL) {
        return;
    }
    free_bst_sf(root->left_child);
    free_bst_sf(root->right_child);
    free(root->mat);
    free(root);
}

matrix_sf* add_mats_sf(const matrix_sf *mat1, const matrix_sf *mat2) {
    if (mat1 == NULL || mat2 == NULL) {
        return NULL;
    }
    // Shapes must match for element-wise addition.
    if (mat1->num_rows != mat2->num_rows || mat1->num_cols != mat2->num_cols) {
        return NULL;
    }
    matrix_sf *result = allocate_matrix(mat1->num_rows, mat1->num_cols);
    if (result == NULL) {
        return NULL;
    }
    unsigned int total = mat1->num_rows * mat1->num_cols;
    for (unsigned int i = 0; i < total; i++) {
        result->values[i] = mat1->values[i] + mat2->values[i];
    }
    return result;
}

matrix_sf* mult_mats_sf(const matrix_sf *mat1, const matrix_sf *mat2) {
    if (mat1 == NULL || mat2 == NULL) {
        return NULL;
    }
    // Inner dimensions must match for multiplication.
    if (mat1->num_cols != mat2->num_rows) {
        return NULL;
    }
    matrix_sf *result = allocate_matrix(mat1->num_rows, mat2->num_cols);
    if (result == NULL) {
        return NULL;
    }
    for (unsigned int row = 0; row < mat1->num_rows; row++) {
        for (unsigned int col = 0; col < mat2->num_cols; col++) {
            long long sum = 0;
            // Dot product of one row and one column.
            for (unsigned int k = 0; k < mat1->num_cols; k++) {
                sum += (long long)mat1->values[row * mat1->num_cols + k] *
                       (long long)mat2->values[k * mat2->num_cols + col];
            }
            result->values[row * mat2->num_cols + col] = (int)sum;
        }
    }
    return result;
}

matrix_sf* transpose_mat_sf(const matrix_sf *mat) {
    if (mat == NULL) {
        return NULL;
    }
    matrix_sf *result = allocate_matrix(mat->num_cols, mat->num_rows);
    if (result == NULL) {
        return NULL;
    }
    // Flip row/column indices.
    for (unsigned int row = 0; row < mat->num_rows; row++) {
        for (unsigned int col = 0; col < mat->num_cols; col++) {
            result->values[col * mat->num_rows + row] = mat->values[row * mat->num_cols + col];
        }
    }
    return result;
}

matrix_sf* create_matrix_sf(char name, const char *expr) {
    if (expr == NULL) {
        return NULL;
    }

    char *cursor = (char *)expr;
    char *endptr = NULL;

    next_token(&cursor);

    // Parse dimensions first.
    unsigned long rows_ul = strtoul(cursor, &endptr, 10);
    if (endptr == cursor) {
        return NULL;
    }
    cursor = endptr;

    unsigned long cols_ul = strtoul(cursor, &endptr, 10);
    if (endptr == cursor) {
        return NULL;
    }
    cursor = endptr;

    unsigned int num_rows = (unsigned int)rows_ul;
    unsigned int num_cols = (unsigned int)cols_ul;
    matrix_sf *mat = allocate_matrix(num_rows, num_cols);
    if (mat == NULL) {
        return NULL;
    }
    mat->name = name;

    // Then parse exactly rows*cols values.
    unsigned int total = num_rows * num_cols;
    for (unsigned int i = 0; i < total; i++) {
        while (*cursor != '\0' && (isspace((unsigned char)*cursor) || *cursor == '[' || *cursor == ';' || *cursor == ']')) {
            cursor++;
        }
        long value = strtol(cursor, &endptr, 10);
        if (endptr == cursor) {
            free(mat);
            return NULL;
        }
        mat->values[i] = (int)value;
        cursor = endptr;
    }

    return mat;
}

static void append_operator(char *postfix, size_t *output_len, char *op_stack, size_t *op_top, char ch) {
    // Pop operators with higher/equal precedence before pushing this one.
    while (*op_top > 0 && op_stack[*op_top - 1] != '(' &&
           matrix_operand_precedence(op_stack[*op_top - 1]) >= matrix_operand_precedence(ch)) {
        postfix[(*output_len)++] = op_stack[--(*op_top)];
    }
    op_stack[(*op_top)++] = ch;
}

char* infix2postfix_sf(char *infix) {
    if (infix == NULL) {
        return NULL;
    }

    size_t len = strlen(infix);
    char *postfix = malloc(len * 2 + 1);
    if (postfix == NULL) {
        return NULL;
    }
    char *op_stack = malloc(len + 1);
    if (op_stack == NULL) {
        free(postfix);
        return NULL;
    }

    size_t output_len = 0;
    size_t op_top = 0;
    for (size_t i = 0; i < len; i++) {
        char ch = infix[i];
        if (isspace((unsigned char)ch)) {
            continue;
        }
        if (ch >= 'A' && ch <= 'Z') {
            // Matrix names go directly to output.
            postfix[output_len++] = ch;
            continue;
        }
        if (ch == '\'') {
            // Transpose is postfix in both infix and postfix forms.
            postfix[output_len++] = ch;
            continue;
        }
        if (ch == '(') {
            op_stack[op_top++] = ch;
            continue;
        }
        if (ch == ')') {
            while (op_top > 0 && op_stack[op_top - 1] != '(') {
                postfix[output_len++] = op_stack[--op_top];
            }
            if (op_top > 0 && op_stack[op_top - 1] == '(') {
                op_top--;
            }
            continue;
        }
        if (ch == '+' || ch == '*') {
            append_operator(postfix, &output_len, op_stack, &op_top, ch);
            continue;
        }
    }

    while (op_top > 0) {
        if (op_stack[op_top - 1] != '(') {
            postfix[output_len++] = op_stack[op_top - 1];
        }
        op_top--;
    }
    postfix[output_len] = '\0';
    free(op_stack);
    return postfix;
}

static matrix_sf *apply_eval_operator(char op, matrix_stack_item_sf *left, matrix_stack_item_sf *right) {
    matrix_sf *result = NULL;
    if (op == '+') {
        result = add_mats_sf(left->mat, right->mat);
    } else if (op == '*') {
        result = mult_mats_sf(left->mat, right->mat);
    }
    // Free temporary operands we owned after combining them.
    if (left->owned) {
        free(left->mat);
    }
    if (right->owned) {
        free(right->mat);
    }
    return result;
}

matrix_sf* evaluate_expr_sf(char name, char *expr, bst_sf *root) {
    if (expr == NULL) {
        return NULL;
    }

    char *postfix = infix2postfix_sf(expr);
    if (postfix == NULL) {
        return NULL;
    }

    size_t expr_len = strlen(postfix);
    matrix_stack_item_sf *stack = malloc((expr_len + 1) * sizeof(matrix_stack_item_sf));
    if (stack == NULL) {
        free(postfix);
        return NULL;
    }

    // Evaluate postfix using a matrix stack.
    size_t top = 0;
    for (size_t i = 0; i < expr_len; i++) {
        char ch = postfix[i];
        if (isspace((unsigned char)ch)) {
            continue;
        }
        if (ch >= 'A' && ch <= 'Z') {
            // Named matrices are looked up from the BST.
            matrix_sf *mat = find_bst_sf(ch, root);
            if (mat == NULL) {
                free_matrix_stack(stack);
                free(postfix);
                return NULL;
            }
            stack[top++] = (matrix_stack_item_sf){.mat = mat, .owned = 0};
            continue;
        }
        if (ch == '\'') {
            // Transpose consumes one operand.
            if (top < 1) {
                free_matrix_stack(stack);
                free(postfix);
                return NULL;
            }
            matrix_stack_item_sf operand = stack[--top];
            matrix_sf *result = transpose_mat_sf(operand.mat);
            if (operand.owned) {
                free(operand.mat);
            }
            if (result == NULL) {
                free_matrix_stack(stack);
                free(postfix);
                return NULL;
            }
            result->name = name;
            stack[top++] = (matrix_stack_item_sf){.mat = result, .owned = 1};
            continue;
        }
        if (ch == '+' || ch == '*') {
            // Binary operators consume two operands.
            if (top < 2) {
                free_matrix_stack(stack);
                free(postfix);
                return NULL;
            }
            matrix_stack_item_sf right = stack[--top];
            matrix_stack_item_sf left = stack[--top];
            matrix_sf *result = apply_eval_operator(ch, &left, &right);
            if (result == NULL) {
                free_matrix_stack(stack);
                free(postfix);
                return NULL;
            }
            result->name = name;
            stack[top++] = (matrix_stack_item_sf){.mat = result, .owned = 1};
        }
    }

    if (top != 1) {
        free_matrix_stack(stack);
        free(postfix);
        return NULL;
    }

    matrix_stack_item_sf final_item = stack[0];
    matrix_sf *result = final_item.mat;
    if (!final_item.owned) {
        // If result points to an existing matrix, clone it for caller ownership.
        result = copy_matrix(result->num_rows, result->num_cols, result->values);
        if (result == NULL) {
            free_matrix_stack(stack);
            free(postfix);
            return NULL;
        }
    }
    result->name = name;

    free_matrix_stack(stack);
    free(postfix);
    return result;
}

static matrix_sf *read_definition_or_expr(char *text, bst_sf *root) {
    // Distinguish raw matrix literals from matrix expressions.
    if (is_matrix_literal_start(text[0])) {
        return create_matrix_sf('?', text);
    }
    return evaluate_expr_sf('?', text, root);
}

static matrix_sf *store_script_result(matrix_sf *mat, bst_sf **root, matrix_sf **result, matrix_sf **last_defined) {
    if (mat == NULL) {
        return NULL;
    }
    // Keep tree and "latest result" pointers in sync.
    *root = insert_bst_sf(mat, *root);
    *last_defined = mat;
    *result = mat;
    return mat;
}

matrix_sf *execute_script_sf(char *filename) {
    if (filename == NULL) {
        return NULL;
    }

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        return NULL;
    }

    bst_sf *root = NULL;
    matrix_sf *result = NULL;
    matrix_sf *last_defined = NULL;
    char *line = NULL;
    size_t line_cap = 0;
    ssize_t line_len;

    // Read script line by line.
    while ((line_len = getline(&line, &line_cap, fp)) != -1) {
        (void)line_len;
        char *trimmed = trim_whitespace(line);
        if (*trimmed == '\0' || *trimmed == '#') {
            continue;
        }

        char *comment = strstr(trimmed, "//");
        if (comment != NULL) {
            // Strip end-of-line comments.
            *comment = '\0';
            trimmed = trim_whitespace(trimmed);
            if (*trimmed == '\0') {
                continue;
            }
        }

        char *equals = strchr(trimmed, '=');
        if (equals != NULL) {
            // Assignment form: X = ...
            *equals = '\0';
            char *lhs = trim_whitespace(trimmed);
            char *rhs = trim_whitespace(equals + 1);
            if (*lhs == '\0' || *rhs == '\0') {
                free(line);
                fclose(fp);
                free_bst_sf(root);
                return NULL;
            }
            char name = lhs[0];
            matrix_sf *mat = read_definition_or_expr(rhs, root);
            if (mat != NULL) {
                mat->name = name;
            }
            if (mat == NULL) {
                free(line);
                fclose(fp);
                free_bst_sf(root);
                return NULL;
            }
            store_script_result(mat, &root, &result, &last_defined);
            continue;
        }

        // Non-assignment line: evaluate it directly.
        matrix_sf *mat = read_definition_or_expr(trimmed, root);
        if (mat == NULL) {
            free(line);
            fclose(fp);
            free_bst_sf(root);
            return NULL;
        }
        store_script_result(mat, &root, &result, &last_defined);
    }

    free(line);
    fclose(fp);

    if (result == NULL) {
        result = last_defined;
    }
    if (result == NULL) {
        free_bst_sf(root);
        return NULL;
    }

    matrix_sf *out = copy_matrix(result->num_rows, result->num_cols, result->values);
    if (out != NULL) {
        // Preserve final matrix name in returned copy.
        out->name = result->name;
    }
    free_bst_sf(root);
    return out;
}

// This is a utility function used during testing. Feel free to adapt the code to implement some of
// the assignment. Feel equally free to ignore it.
matrix_sf *copy_matrix(unsigned int num_rows, unsigned int num_cols, int values[]) {
    matrix_sf *m = malloc(sizeof(matrix_sf)+num_rows*num_cols*sizeof(int));
    m->name = '?';
    m->num_rows = num_rows;
    m->num_cols = num_cols;
    memcpy(m->values, values, num_rows*num_cols*sizeof(int));
    return m;
}

// Don't touch this function. It's used by the testing framework.
// It's been left here in case it helps you debug and test your code.
void print_matrix_sf(matrix_sf *mat) {
    assert(mat != NULL);
    assert(mat->num_rows <= 1000);
    assert(mat->num_cols <= 1000);
    printf("%d %d ", mat->num_rows, mat->num_cols);
    for (unsigned int i = 0; i < mat->num_rows*mat->num_cols; i++) {
        printf("%d", mat->values[i]);
        if (i < mat->num_rows*mat->num_cols-1)
            printf(" ");
    }
    printf("\n");
}
