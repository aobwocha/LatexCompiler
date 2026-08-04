package main

import (
	"fmt"
	"os"
	"strings"
	"unicode"
)

type TokenType int

const (
	TokenCommand TokenType = iota
	TokenLBrace
	TokenRBrace
	TokenText
)

type Position struct {
	Line int
	Col  int
}

type Token struct {
	Type     TokenType
	Value    string
	StartIdx int
	EndIdx   int
	StartPos Position
	EndPos   Position
}

func main() {
	if len(os.Args) < 2 {
		fmt.Println("Please provide a LaTeX string to parse.")
		return
	}

	latex := strings.Join(os.Args[1:], " ")
	var tokens []Token

	charIdx := 0
	line := 1
	col := 1

	advance := func() {
		if charIdx < len(latex) {
			if latex[charIdx] == '\n' {
				line++
				col = 1
			} else {
				col++
			}
			charIdx++
		}
	}

	for charIdx < len(latex) {
		startIdx := charIdx
		startPos := Position{Line: line, Col: col}
		currentChar := latex[charIdx]

		switch currentChar {
		case '{':
			advance()
			tokens = append(tokens, Token{
				Type:     TokenLBrace,
				Value:    "{",
				StartIdx: startIdx,
				EndIdx:   charIdx,
				StartPos: startPos,
				EndPos:   Position{Line: line, Col: col},
			})

		case '}':
			advance()
			tokens = append(tokens, Token{
				Type:     TokenRBrace,
				Value:    "}",
				StartIdx: startIdx,
				EndIdx:   charIdx,
				StartPos: startPos,
				EndPos:   Position{Line: line, Col: col},
			})

		case '\\':
			advance() // Consume '\'

			if charIdx < len(latex) {
				if unicode.IsLetter(rune(latex[charIdx])) {
					for charIdx < len(latex) && unicode.IsLetter(rune(latex[charIdx])) {
						advance()
					}
				} else {
					advance()
				}
			}

			tokens = append(tokens, Token{
				Type:     TokenCommand,
				Value:    latex[startIdx:charIdx],
				StartIdx: startIdx,
				EndIdx:   charIdx,
				StartPos: startPos,
				EndPos:   Position{Line: line, Col: col},
			})

		default:
			for charIdx < len(latex) && latex[charIdx] != '\\' && latex[charIdx] != '{' && latex[charIdx] != '}' {
				advance()
			}

			tokens = append(tokens, Token{
				Type:     TokenText,
				Value:    latex[startIdx:charIdx],
				StartIdx: startIdx,
				EndIdx:   charIdx,
				StartPos: startPos,
				EndPos:   Position{Line: line, Col: col},
			})
		}
	}

	// Print tokens with line and column ranges
	for i, t := range tokens {
		fmt.Printf("[%d] Type: %d | Value: %-15q | Pos: Line %d:%d -> Line %d:%d\n",
			i, t.Type, t.Value, t.StartPos.Line, t.StartPos.Col, t.EndPos.Line, t.EndPos.Col)
	}
}
