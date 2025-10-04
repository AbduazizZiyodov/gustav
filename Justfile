DEBUG := "yes"
SCRIPT_DIR := "./scripts"

default: clean check test

repl:
	DEBUG={{DEBUG}} {{SCRIPT_DIR}}/gus

check:
	uv run ruff format .
	uv run ruff check .
	uv run mypy --strict --ignore-missing-imports --explicit-package-bases .

generate_ast:
	{{SCRIPT_DIR}}/generate_ast

test:
	uv run pytest -s -vv \
		--cov=gustav \
		--cov-report=term-missing \
		--cov-report=html \
		testing

test-no-cover:
	uv run pytest -s -vv testing

clean:
	py3clean . && rm -rf htmlcov __pycache__ .mypy_cache .ruff_cache .coverage.*