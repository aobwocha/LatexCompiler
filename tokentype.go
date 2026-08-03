type TokenType int

const (
    TokenCommand   TokenType = iota
    TokenLBrace
    TokenRBrace
    TokenText
    TokenEOF
)

type Position struct {
    Line   int
    Column int
    Offset int
}

type Token struct {
    Type     TokenType
    Value    string
    PosStart Position
    PosEnd   Position
}
