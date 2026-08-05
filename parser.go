package main

import (
	"fmt"
)

type Node interface {
	node()
}

type TextNode struct {
	Value string
}

func (t *TextNode) node() {}

type GroupNode struct {
	Children []Node
}

func (g *GroupNode) node() {}

type CommandNode struct {
	Name    string
	OptArgs []Node
	ReqArgs []Node
}

func (c *CommandNode) node() {}

type Parser struct {
	tokens []Token
	pos    int
}

func NewParser(tokens []Token) *Parser {
	return &Parser{tokens: tokens, pos: 0}
}

func (p *Parser) peek() Token {
	if p.pos >= len(p.tokens) {
		return Token{Type: TokenText, Value: ""}
	}
	return p.tokens[p.pos]
}

func (p *Parser) advance() Token {
	tok := p.peek()
	if p.pos < len(p.tokens) {
		p.pos++
	}
	return tok
}

func (p *Parser) consume(expected TokenType) (Token, error) {
	tok := p.peek()
	if tok.Type != expected {
		return tok, fmt.Errorf("expected token type %d at pos %d, got %d (%q)",
			expected, p.pos, tok.Type, tok.Value)
	}
	return p.advance(), nil
}

func (p *Parser) isAtEnd() bool {
	return p.pos >= len(p.tokens)
}

func (p *Parser) parseNodeList() []Node {
	var nodes []Node

	for !p.isAtEnd() {
		tokType := p.peek().Type

		if tokType == TokenRBrace || tokType == TokenRBracket {
			break
		}

		switch tokType {
		case TokenCommand:
			nodes = append(nodes, p.parseCommand())
		case TokenLBrace:
			nodes = append(nodes, p.parseGroup())
		default:
			nodes = append(nodes, p.parseText())
		}
	}

	return nodes
}

func (p *Parser) parseCommand() *CommandNode {
	cmdTok := p.advance()
	cmdNode := &CommandNode{
		Name: cmdTok.Value,
	}

	if !p.isAtEnd() && p.peek().Type == TokenLBracket {
		p.consume(TokenLBracket)
		cmdNode.OptArgs = p.parseNodeList()
		p.consume(TokenRBracket)
	}

	if !p.isAtEnd() && p.peek().Type == TokenLBrace {
		p.consume(TokenLBrace)
		cmdNode.ReqArgs = p.parseNodeList()
		p.consume(TokenRBrace)
	}

	return cmdNode
}

func (p *Parser) parseGroup() *GroupNode {
	p.consume(TokenLBrace)
	children := p.parseNodeList()
	p.consume(TokenRBrace)

	return &GroupNode{Children: children}
}

func (p *Parser) parseText() *TextNode {
	tok := p.advance()
	return &TextNode{Value: tok.Value}
}
