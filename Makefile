test:
	DEBUG=yes ./.venv/bin/python3.13 -m gustav examples/file.gus

check:
	ruff format && mypy --strict --ignore-missing-imports --explicit-package-bases .

repl:
	DEBUG=yes ./.venv/bin/python3.13 -m gustav