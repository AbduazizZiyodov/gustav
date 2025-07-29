DEBUG := "yes"
TARGET := "examples/file.gus"
SCRIPT_DIR := "./scripts"

default: clean check test

run target="":
	#!/usr/bin/env bash
	if [[ -z "$target" ]]; then
	  DEBUG={{DEBUG}} {{SCRIPT_DIR}}/gus {{TARGET}}
	else
	  DEBUG={{DEBUG}} {{SCRIPT_DIR}}/gus "$target"
	fi

repl:
	DEBUG={{DEBUG}} {{SCRIPT_DIR}}/gus

check:
	ruff format .
	ruff check .
	mypy --strict --ignore-missing-imports --explicit-package-bases .

generate_ast:
	{{SCRIPT_DIR}}/generate_ast

test:
	pytest -s -vv \
		--cov=gustav \
		--cov-report=term-missing \
		--cov-report=html \
		testing

clean:
	py3clean . && rm -rf htmlcov __pycache__ .mypy_cache .ruff_cache .coverage.*