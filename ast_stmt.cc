/* File: ast_stmt.cc
 * -----------------
 * Implementation of statement node classes.
 */
#include "ast_stmt.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "ast_expr.h"
#include "assert.h"

#ifdef DEBUG
List<List<Decl*>*> *debugScope = NULL;
#endif

Program::Program(List<Decl*> *d) : scope(new List<List<Decl*>*>) {
    Assert(d != NULL);
    (decls=d)->SetParentAll(this);

    scope->Append(new List<Decl*>); // Add initial empty global scope

#ifdef DEBUG
    debugScope = scope;
#endif
}

void Program::Check() {
    /* pp3: here is where the semantic analyzer is kicked off.
     *      The general idea is perform a tree traversal of the
     *      entire program, examining all constructs for compliance
     *      with the semantic rules.  Each node can have its own way of
     *      checking itself, which makes for a great use of inheritance
     *      and polymorphism in the node classes.
     */
    for (int i = 0, numElems = decls->NumElements(); i < numElems; i++)
        decls->Nth(i)->Check(scope);
}

#ifdef DEBUG
void Program::PrintScope() {
    for (int i = 0, numScopes = debugScope->NumElements(); i < numScopes; ++i) {
        std::cout << "========== Scope ==========" << std::endl;

        List<Decl*> *s = debugScope->Nth(i);
        for (int j = 0, numElems = s->NumElements(); j < numElems; ++j)
            std::cout << s->Nth(j) << std::endl;
    }
}
#endif

void Stmt::Check(List<List<Decl*>*> *scopeList) {
    /* This method should not be called. Any Check calls should fall through
     * to each subclasses Check method.
     */
    assert(0);
    return;
}

StmtBlock::StmtBlock(List<VarDecl*> *d, List<Stmt*> *s) {
    Assert(d != NULL && s != NULL);
    (decls=d)->SetParentAll(this);
    (stmts=s)->SetParentAll(this);
}

ConditionalStmt::ConditionalStmt(Expr *t, Stmt *b) {
    Assert(t != NULL && b != NULL);
    (test=t)->SetParent(this);
    (body=b)->SetParent(this);
}

ForStmt::ForStmt(Expr *i, Expr *t, Expr *s, Stmt *b): LoopStmt(t, b) {
    Assert(i != NULL && t != NULL && s != NULL && b != NULL);
    (init=i)->SetParent(this);
    (step=s)->SetParent(this);
}

IfStmt::IfStmt(Expr *t, Stmt *tb, Stmt *eb): ConditionalStmt(t, tb) {
    Assert(t != NULL && tb != NULL); // else can be NULL
    elseBody = eb;
    if (elseBody) elseBody->SetParent(this);
}

ReturnStmt::ReturnStmt(yyltype loc, Expr *e) : Stmt(loc) {
    Assert(e != NULL);
    (expr=e)->SetParent(this);
}

PrintStmt::PrintStmt(List<Expr*> *a) {
    Assert(a != NULL);
    (args=a)->SetParentAll(this);
}
