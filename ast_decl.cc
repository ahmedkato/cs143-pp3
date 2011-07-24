/* File: ast_decl.cc
 * -----------------
 * Implementation of Decl node classes.
 */
#include "ast_decl.h"
#include "ast_type.h"
#include "ast_stmt.h"

Decl::Decl(Identifier *n) : Node(*n->GetLocation()), scope(new Scope) {
    Assert(n != NULL);
    (id=n)->SetParent(this);
}

bool Decl::IsEquivalentTo(Decl *other) {
    /* TODO: Once all subclasses support this function it should be made a pure
     * virtual function.
     */
    return true;
}

void Decl::BuildScope(Scope *parent) {
    /* TODO: Once all subclasses support this function it should be made a pure
     * virtual function.
     */
}

void Decl::Check() {
    /* TODO: Once all subclasses support this function it should be made a pure
     * virtual function.
     */
}

VarDecl::VarDecl(Identifier *n, Type *t) : Decl(n) {
    Assert(n != NULL && t != NULL);
    (type=t)->SetParent(this);
}

bool VarDecl::IsEquivalentTo(Decl *other) {
    VarDecl *varDecl = dynamic_cast<VarDecl*>(other);
    if (varDecl == NULL)
        return false;

    return type->IsEquivalentTo(varDecl->type);
}

void VarDecl::BuildScope(Scope *parent) {
    /* VarDecls don't create a new scope of their own */
    delete scope;
    scope = parent;
}

void VarDecl::Check() {
    CheckType();
}

void VarDecl::CheckType() {
    if (type->IsPrimitive())
        return;

    Scope *s = scope;
    while (s != NULL) {
        if (s->table->Lookup(type->Name()) != NULL)
            return;
        s = s->GetParent();
    }

    type->ReportNotDeclaredIdentifier(LookingForType);
}

ClassDecl::ClassDecl(Identifier *n, NamedType *ex, List<NamedType*> *imp, List<Decl*> *m) : Decl(n) {
    // extends can be NULL, impl & mem may be empty lists but cannot be NULL
    Assert(n != NULL && imp != NULL && m != NULL);
    extends = ex;
    if (extends) extends->SetParent(this);
    (implements=imp)->SetParentAll(this);
    (members=m)->SetParentAll(this);
}

void ClassDecl::BuildScope(Scope *parent) {
    scope->SetParent(parent);

    for (int i = 0, n = members->NumElements(); i < n; ++i)
        scope->AddDecl(members->Nth(i));

    for (int i = 0, n = members->NumElements(); i < n; ++i)
        members->Nth(i)->BuildScope(scope);
}

void ClassDecl::Check() {
    CheckExtends();
    CheckImplements();
}

void ClassDecl::CheckExtends() {
    if (extends == NULL)
        return;

    Decl *lookup = scope->GetParent()->table->Lookup(extends->Name());
    ClassDecl *extDecl = dynamic_cast<ClassDecl*>(lookup);

    if (extDecl == NULL)
        extends->ReportNotDeclaredIdentifier(LookingForClass);
    else
        CheckExtendedMembers(extDecl);
}

void ClassDecl::CheckImplements() {
    Scope *s = scope->GetParent();

    for (int i = 0, n = implements->NumElements(); i < n; ++i) {
        NamedType *nth = implements->Nth(i);
        Decl *lookup = s->table->Lookup(nth->Name());
        InterfaceDecl *intDecl = dynamic_cast<InterfaceDecl*>(lookup);

        if (intDecl == NULL)
            nth->ReportNotDeclaredIdentifier(LookingForInterface);
        else
            CheckImplementedMembers(intDecl);
    }
}

void ClassDecl::CheckExtendedMembers(ClassDecl *extDecl) {
    if (extDecl != NULL && extDecl->extends != NULL) {
        Scope *s = extDecl->scope->GetParent();
        Decl *lookup = s->table->Lookup(extDecl->extends->Name());
        CheckExtendedMembers(dynamic_cast<ClassDecl*>(lookup));
    }

    CheckAgainstScope(extDecl->scope);
}

void ClassDecl::CheckImplementedMembers(InterfaceDecl *intDecl) {
    CheckAgainstScope(intDecl->GetScope());
}

void ClassDecl::CheckAgainstScope(Scope *other) {
    Iterator<Decl*> iter = scope->table->GetIterator();
    Decl *d;
    while ((d = iter.GetNextValue()) != NULL) {
        Decl *lookup = other->table->Lookup(d->Name());

        if (lookup != NULL && !d->IsEquivalentTo(lookup))
            ReportError::OverrideMismatch(d);
    }
}

InterfaceDecl::InterfaceDecl(Identifier *n, List<Decl*> *m) : Decl(n) {
    Assert(n != NULL && m != NULL);
    (members=m)->SetParentAll(this);
}

void InterfaceDecl::BuildScope(Scope *parent) {
    scope->SetParent(parent);

    for (int i = 0, n = members->NumElements(); i < n; ++i)
        scope->AddDecl(members->Nth(i));

    for (int i = 0, n = members->NumElements(); i < n; ++i)
        members->Nth(i)->BuildScope(scope);
}

FnDecl::FnDecl(Identifier *n, Type *r, List<VarDecl*> *d) : Decl(n) {
    Assert(n != NULL && r!= NULL && d != NULL);
    (returnType=r)->SetParent(this);
    (formals=d)->SetParentAll(this);
    body = NULL;
}

void FnDecl::SetFunctionBody(Stmt *b) {
    (body=b)->SetParent(this);
}

bool FnDecl::IsEquivalentTo(Decl *other) {
    FnDecl *fnDecl = dynamic_cast<FnDecl*>(other);

    if (fnDecl == NULL)
        return false;

    if (!returnType->IsEquivalentTo(fnDecl->returnType))
        return false;

    if (formals->NumElements() != fnDecl->formals->NumElements())
        return false;

    for (int i = 0, n = formals->NumElements(); i < n; ++i)
        if (!formals->Nth(i)->IsEquivalentTo(fnDecl->formals->Nth(i)))
            return false;

    return true;
}

void FnDecl::BuildScope(Scope *parent) {
    scope->SetParent(parent);

    for (int i = 0, n = formals->NumElements(); i < n; ++i)
        scope->AddDecl(formals->Nth(i));

    for (int i = 0, n = formals->NumElements(); i < n; ++i)
        formals->Nth(i)->BuildScope(scope);

    if (body)
        body->BuildScope(scope);
}

void FnDecl::Check() {
    for (int i = 0, n = formals->NumElements(); i < n; ++i)
        formals->Nth(i)->Check();

    if (body)
        body->Check();
}
