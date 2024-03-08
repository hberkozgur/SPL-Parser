#include <iostream>
#include <string>
#include <stack>
#include <map>
#include "lex.h"
#include "parser.h"
using namespace std;

namespace Parser {

    bool pushed_back = false;
    LexItem pushed_token;

    static LexItem GetNextToken(istream& in, int& line) {
        if (pushed_back) {
            pushed_back = false;
            return pushed_token;
        }
        return getNextToken(in, line);
    }

    static void PushBackToken(LexItem& t) {
        if (pushed_back) {
             abort();
        }
        pushed_back = true;
        pushed_token = t;
    }
    map<string, bool> defVar;
    
}
    static int error_count = 0;
    int ErrCount() {
        return error_count;
    }

    void ParseError(int line, string msg){
        ++error_count;
        cout <<error_count<< ". "<< "Line # " << line << ": " << msg << endl;
    }

    
    



    

bool Prog(istream& in, int& line) {
    bool success = StmtList(in, line);

    LexItem item = Parser::GetNextToken(in, line);
    if (item.GetToken() != DONE) {
        ParseError(line, "Missing Program");
        success = false;
    }
    else{
        cout<<"(DONE)"<<endl;
    }
    return success;
}


bool StmtList(istream& in, int& line) {
    if (!Stmt(in, line)) {
        return false;
    }

    while (true) {
        LexItem tok = Parser::GetNextToken(in, line);
        if (tok != SEMICOL) {
            Parser::PushBackToken(tok);
            break;
        }

        // Check if the next token is a RBRACES
        LexItem nextTok = Parser::GetNextToken(in, line);
        if (nextTok == RBRACES) {
            Parser::PushBackToken(nextTok);
            return true;
        } else {
            Parser::PushBackToken(nextTok);
        }

        if(!Stmt(in, line)) {
            ParseError(line, "Syntactic error in Program Body.");
            return false;
        }
    }
    LexItem tok = Parser::GetNextToken(in, line);
    if (tok != DONE) {
        ParseError(line, "Missing semicolon at end of Statement.");
        return false;
    }
    else{
        return true;
    }
}






bool Stmt(istream& in, int& line) {
    LexItem tok = Parser::GetNextToken(in, line);
    bool success = true;

    switch (tok.GetToken()) {
        case IDENT:
        case SIDENT:
        case NIDENT:
            Parser::PushBackToken(tok);
            success = AssignStmt(in, line);
            if (!success) {
                ParseError(line, "Incorrect Assignment Statement.");
                return false;
            }
            break;

        case WRITELN:
            success = WritelnStmt(in, line);
            if (!success) {
                ParseError(line, "Incorrect Writeln Statement.");
                return false;
            }
            break;

        case IF:
            success = IfStmt(in, line);
            break;
            


        default:
            Parser::PushBackToken(tok);
            success = true;

    }

    return success;
}

bool SimpleStmt(istream& in, int& line){
    return true;
}

bool SimpleIfStmt(istream& in, int& line){
    return true;
}

bool WritelnStmt(istream& in, int& line) {
    LexItem t;
    t = Parser::GetNextToken(in, line);
    if (t != LPAREN) {
        ParseError(line, "Missing Left Parenthesis of Writeln Statement");
        return false;
    }
    bool ex = ExprList(in, line);
    if (!ex) {
        ParseError(line, "Missing expression after PRINT");
        return false;
    }
    t = Parser::GetNextToken(in, line);
    if (t != RPAREN) {
        ParseError(line, "Missing Right Parenthesis of Writeln Statement");
        return false;
    }
    return ex;
}

bool IfStmt(istream& in, int& line) {
    LexItem t;
    bool ex;

    t = Parser::GetNextToken(in, line);
    if (t != LPAREN) {
        ParseError(line, "Missing Left Parenthesis");
        return false;
    }

    ex = Expr(in, line);

    if (!ex) {
        ParseError(line, "Missing expression after if");
        return false;
    }

    t = Parser::GetNextToken(in, line);
    if (t != RPAREN) {
        ParseError(line, "Missing Right Parenthesis");
        return false;
    }

    t = Parser::GetNextToken(in, line);
    if (t != LBRACES) {
        ParseError(line, "Missing Left Brace after if");
        return false;
    }

    bool sl = StmtList(in, line);
    if (!sl) {
        ParseError(line, "Missing statements in if clause");
        return false;
    }
    t = Parser::GetNextToken(in, line);
    if (t != RBRACES) {
        ParseError(line, "Missing Right Brace after if");
        return false;
    }

    t = Parser::GetNextToken(in, line);
    if (t == ELSE) {
        t = Parser::GetNextToken(in, line);
        if (t != LBRACES) {
            ParseError(line, "Missing Left Brace after else");
            return false;
        }

        bool sl = StmtList(in, line);
        if (!sl) {
            ParseError(line, "Missing statements in else clause");
            return false;
        }

        t = Parser::GetNextToken(in, line);
        if (t != RBRACES) {
            ParseError(line, "Missing Right Brace after else");
            return false;
        }
    } else {
        Parser::PushBackToken(t);
    }

    return true;
}


bool AssignStmt(istream& in, int& line) {
    bool ex = Var(in, line);
    if (!ex) {
        ParseError(line, "Missing ");
        return false;
    }

    // Parse the equal sign
    LexItem eq = Parser::GetNextToken(in, line);
    if (eq != ASSOP) {
        Parser::PushBackToken(eq);
        ParseError(line, "Missing operand after operator");
        return false;
    }

    // Parse the expression on the right-hand side of the assignment
    ex = Expr(in, line);
    if (!ex) {
        ParseError(line, "Missing Expression in Assignment Statement");
        
        return false;
    }

    // Return success
    return true;
}


bool Var(istream& in, int& line) {
        // Parse the variable on the left-hand side of the assignment
    LexItem var = Parser::GetNextToken(in, line);
    if (var != NIDENT && var != SIDENT) {
        Parser::PushBackToken(var);
        ParseError(line, "Missing identifier in assignment statement");
        return false;
    }
    if (!Parser::defVar.count(var.GetLexeme())) {
        Parser::defVar[var.GetLexeme()] = true;
    }

    return true;

}


bool ExprList(istream& in, int& line) {
    bool status = false;
    status = Expr(in, line);
    if (!status) {
        ParseError(line, "Missing Expression");
        return false;
    }
    LexItem tok = Parser::GetNextToken(in, line);
    if (tok == COMMA) {
        status = ExprList(in, line);
    }
    else if (tok.GetToken() == ERR) {
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }
    else {
        Parser::PushBackToken(tok);
    }
    return true;
}


bool Expr(istream& in, int& line) {
    bool status = RelExpr(in, line);
    if (!status) {
        return false;
    }

    LexItem tok = Parser::GetNextToken(in, line);
    if (tok == NEQ || tok == SEQ) {
        status = RelExpr(in, line);
        if (!status) {
            ParseError(line, "Missing right-hand side of comparison");
            return false;
        }
    } else {
        Parser::PushBackToken(tok);
    }


    return status;
}


bool RelExpr(istream& in, int& line) {
    bool status = AddExpr(in, line);
    if (!status) {
        return false;
    }
    LexItem tok = Parser::GetNextToken(in, line);
    if (tok == NLTHAN || tok == NGTHAN || tok == SLTHAN || tok ==SGTHAN) {
        status = AddExpr(in, line);
        if (!status) {
            ParseError(line, "Missing expression after comparison operator");
            return false;
        }
    } else {
        Parser::PushBackToken(tok);
    }
    return true;

}


bool AddExpr(istream& in, int& line) {
    bool status = MultExpr(in, line);
    if (!status) {
        return false;
    }
    while (true) {
        LexItem op = Parser::GetNextToken(in, line);
        if (op == PLUS || op == MINUS || op == CAT) {
            status = MultExpr(in, line);
            if (!status) {
                ParseError(line, "Missing operand after operator");
                return false;
            }
        }
        else {
            Parser::PushBackToken(op);
            break;
        }
    }
    return true;
}

bool MultExpr(istream& in, int& line) {
    bool success = ExponExpr(in, line);
    if (!success) {
        return false;
    }
    
    LexItem t = Parser::GetNextToken(in, line);

    while (t == MULT || t == DIV || t == SREPEAT) {
    bool sub_success = ExponExpr(in, line);

    if (!sub_success) {
        success = false;
        return false;
    }

    t = Parser::GetNextToken(in, line);
}

Parser::PushBackToken(t);
return true;

}

bool ExponExpr(istream& in, int& line) {
    bool status = UnaryExpr(in, line);
    if (!status) {
        return false;
    }
    LexItem tok = Parser::GetNextToken(in, line);
    while (tok == EXPONENT) {
        status = UnaryExpr(in, line);
        if (!status) {
            ParseError(line, "Missing operand after ^");
            return false;
        }
        tok = Parser::GetNextToken(in, line);
    }
    Parser::PushBackToken(tok);
    return true;

}
bool UnaryExpr(istream& in, int& line) {
    LexItem t = Parser::GetNextToken(in, line);
    int sign;
    if (t == MINUS) {
        sign = -1;
    }
    else if ( t == PLUS){
        sign = 1;

    }
    else{
        Parser::PushBackToken(t);
    }    
    

    bool pe = PrimaryExpr(in, line, sign);
    if (!pe) {
        return false;
    }


    return true;
}

bool PrimaryExpr(istream& in, int& line, int sign) {
    LexItem t = Parser::GetNextToken(in, line);

    if (t == IDENT) {
        if (Parser::defVar.find(t.GetLexeme()) == Parser::defVar.end()) {
            ParseError(line, "Undefined Variable"+t.GetLexeme());
            return false;
        }
        if (Parser::defVar[t.GetLexeme()] == false) {
            ParseError(line, "Variable Used before being set");
            return false;
        }
        return true;
    }

    if (t == SIDENT || t == NIDENT) {
        if (Parser::defVar.find(t.GetLexeme()) == Parser::defVar.end()) {
            ParseError(line, "Using Undefined Variable");
            return false;
        }
        if (Parser::defVar[t.GetLexeme()] == false) {
            ParseError(line, "Variable Used before being set");
            return false;
        }
        return true;
    }

    if (t == ICONST || t == RCONST || t == SCONST) {
        if (sign == -1 && t == SCONST) {
            ParseError(line, "Unary operator '-' cannot be used with string");
            return false;
        }
        return true;
    }

    if (t == LPAREN) {
        bool ex = Expr(in, line);
        if (!ex) {
            ParseError(line, "Missing Expression after Left Parenthesis");
            return false;
        }
        t = Parser::GetNextToken(in, line);
        if (t != RPAREN) {
            ParseError(line, "Missing right Parenthesis after expression");
            return false;
        }
        return true;
    }
    if(t == SEMICOL){
        ParseError(line, "Missing operand after operator");
        return false; 
    }
    else{
        ParseError(line, "Missing operand after operator");
        return false;
    }

    Parser::PushBackToken(t);
    ParseError(line, "Missing Primary Expression: "+t.GetLexeme());
    return false;
}

