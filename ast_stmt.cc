/* File: ast_stmt.cc
 * -----------------
 * Implementation of statement node classes.
 */
#include "ast_stmt.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "ast_expr.h"
#include "errors.h"
#include "ast_type.h"

int Scope::AddDecl(Decl *d) {
    Decl *lookup = table->Lookup(d->Name());

    if (lookup != NULL) {
            ReportError::DeclConflict(d, lookup);
            return 1;
    }

    table->Enter(d->Name(), d);
    return 0;
}

ostream& operator<<(ostream& out, Scope *s) {
    out << "========== Scope ==========" << std::endl;
    Iterator<Decl*> iter = s->table->GetIterator();
    Decl *d;
    while ((d = iter.GetNextValue()) != NULL)
        out << d << std::endl;
    return out;
}

Scope *Program::gScope = new Scope();

Program::Program(List<Decl*> *d) {
    Assert(d != NULL);
    (decls=d)->SetParentAll(this);
}

void Program::Check() {
    /* pp3: here is where the semantic analyzer is kicked off.
     *      The general idea is perform a tree traversal of the
     *      entire program, examining all constructs for compliance
     *      with the semantic rules.  Each node can have its own way of
     *      checking itself, which makes for a great use of inheritance
     *      and polymorphism in the node classes.
     */

    BuildScope();

    for (int i = 0, n = decls->NumElements(); i < n; ++i)
        decls->Nth(i)->Check();
}

void Program::BuildScope() {
    for (int i = 0, n = decls->NumElements(); i < n; ++i)
        gScope->AddDecl(decls->Nth(i));

    for (int i = 0, n = decls->NumElements(); i < n; ++i)
        decls->Nth(i)->BuildScope(gScope);
}

void Stmt::BuildScope(Scope *parent) {
    scope->SetParent(parent);
}

void Stmt::Check() {
    /* TODO: Once all sublcasses support this function it should be made a pure
     * virtual function.
     */
}

StmtBlock::StmtBlock(List<VarDecl*> *d, List<Stmt*> *s) {
    Assert(d != NULL && s != NULL);
    (decls=d)->SetParentAll(this);
    (stmts=s)->SetParentAll(this);
}

void StmtBlock::BuildScope(Scope *parent) {
    scope->SetParent(parent);

    for (int i = 0, n = decls->NumElements(); i < n; ++i)
        scope->AddDecl(decls->Nth(i));

    for (int i = 0, n = decls->NumElements(); i < n; ++i)
        decls->Nth(i)->BuildScope(scope);

    for (int i = 0, n = stmts->NumElements(); i < n; ++i)
        stmts->Nth(i)->BuildScope(scope);
}

void StmtBlock::Check() {
    for (int i = 0, n = decls->NumElements(); i < n; ++i)
        decls->Nth(i)->Check();

    for (int i = 0, n = stmts->NumElements(); i < n; ++i)
        stmts->Nth(i)->Check();
}

ConditionalStmt::ConditionalStmt(Expr *t, Stmt *b) {
    Assert(t != NULL && b != NULL);
    (test=t)->SetParent(this);
    (body=b)->SetParent(this);
}

void ConditionalStmt::BuildScope(Scope *parent) {
    scope->SetParent(parent);

    test->BuildScope(scope);
    body->BuildScope(scope);
}

void ConditionalStmt::Check() {
    test->Check();
    body->Check();

    if (!test->GetType()->IsEquivalentTo(Type::boolType))
        ReportError::TestNotBoolean(test);
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
