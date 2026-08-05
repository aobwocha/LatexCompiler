package main

import (
	"fmt"
	"os"
	"path/filepath"
	"unicode"
)

type TokenType int

const (
	TokenCommand TokenType = iota
	TokenLBrace
	TokenRBrace
	TokenLBracket
	TokenRBracket
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

func Lex(filePath string) ([]Token, error) {
	if filepath.Ext(filePath) != ".tex" {
		fmt.Printf("Warning: File %q does not have a .tex extension\n", filePath)
	}

	fileBytes, err := os.ReadFile(filePath)
	if err != nil {
		return nil, fmt.Errorf("error reading file %q: %w", filePath, err)
	}

	latex := string(fileBytes)
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

		case '[':
			advance()
			tokens = append(tokens, Token{
				Type:     TokenLBracket,
				Value:    "[",
				StartIdx: startIdx,
				EndIdx:   charIdx,
				StartPos: startPos,
				EndPos:   Position{Line: line, Col: col},
			})

		case ']':
			advance()
			tokens = append(tokens, Token{
				Type:     TokenRBracket,
				Value:    "]",
				StartIdx: startIdx,
				EndIdx:   charIdx,
				StartPos: startPos,
				EndPos:   Position{Line: line, Col: col},
			})

		case '\\':
			advance()

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
			for charIdx < len(latex) {
				c := latex[charIdx]
				if c == '\\' || c == '{' || c == '}' || c == '[' || c == ']' {
					break
				}
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

	return tokens, nil
}
