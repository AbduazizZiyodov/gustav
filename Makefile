example:
	DEBUG=yes ./scripts/gus examples/file.gus

check:
	ruff format && mypy --strict --ignore-missing-imports --explicit-package-bases .

repl:
	DEBUG=yes ./scripts/gus

generate_ast:
	./scripts/generate_ast