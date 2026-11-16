DEBUG := "yes"

default: clean check

check:
	uv run ruff format gustav/ tools/
	uv run ruff check --fix gustav/ tools/
	uv run mypy --strict --ignore-missing-imports --explicit-package-bases gustav/ tools/

generate-ast:
	./.venv/bin/python3.15 -m tools generate_ast

clean:
	py3clean . && rm -rf htmlcov __pycache__ .mypy_cache .ruff_cache .coverage.*
