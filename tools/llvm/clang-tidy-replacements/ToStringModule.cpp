#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang-tidy/ClangTidy.h"
#include "clang-tidy/ClangTidyModule.h"
#include "clang-tidy/ClangTidyCheck.h"
#include "clang/Lex/Lexer.h"

using namespace clang::ast_matchers;
using namespace clang;

namespace {

class ToStringCheck : public tidy::ClangTidyCheck {
public:
  ToStringCheck(StringRef Name, tidy::ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}

  void registerMatchers(MatchFinder *Finder) override {
    Finder->addMatcher(
        cxxMemberCallExpr(
            callee(cxxMethodDecl(
                hasName("to_string"),
                ofClass(hasName("boost::basic_string_view"))
            )),
            on(expr().bind("object"))
        ).bind("call"),
        this);
  }

  void check(const MatchFinder::MatchResult &Result) override {
    const auto *Call =
        Result.Nodes.getNodeAs<CXXMemberCallExpr>("call");
    const auto *Object =
        Result.Nodes.getNodeAs<Expr>("object");

    if (!Call || !Object)
      return;

    const auto &SM = *Result.SourceManager;
    const auto &LO = Result.Context->getLangOpts();

    const Expr *Base = Object->IgnoreImpCasts();

    // Extract source text of the object
    CharSourceRange ObjRange =
        CharSourceRange::getTokenRange(Base->getSourceRange());

    StringRef ObjTextRef =
        Lexer::getSourceText(ObjRange, SM, LO);

    if (ObjTextRef.empty())
      return;

    std::string ObjText = ObjTextRef.str();

    // Handle pointer case: ptr->to_string() → *ptr
    if (Base->getType()->isPointerType()) {
      ObjText = "*" + ObjText;
    }

    // Replace entire call expression
    CharSourceRange CallRange =
        CharSourceRange::getTokenRange(Call->getSourceRange());

    std::string Replacement =
        "std::string{ " + ObjText + " }";

    diag(Call->getBeginLoc(),
         "replace boost::string_view::to_string() with std::string{...}")
        << FixItHint::CreateReplacement(CallRange, Replacement);
  }
};

class ToStringModule : public clang::tidy::ClangTidyModule {
public:
  void addCheckFactories(
      clang::tidy::ClangTidyCheckFactories &Factories) override {
    Factories.registerCheck<ToStringCheck>(
        "custom-to-string");
  }
};

} // namespace

// Register the module
static clang::tidy::ClangTidyModuleRegistry::Add<ToStringModule>
    X("to-string-module", "Adds to_string rewrite check.");

// Required for clang-tidy to recognize the module
volatile int ToStringModuleAnchorSource = 0;
