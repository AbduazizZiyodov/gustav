import os
import logging

__all__ = ["LOG"]

DEBUG: bool = os.getenv("DEBUG", default=str()).lower() in ["y", "true", "yes"]

logging.basicConfig(
    level=logging.DEBUG if DEBUG else logging.INFO,
    format="[%(levelname)s | %(funcName)s():%(lineno)d] %(message)s",
)


LOG = logging.getLogger(__name__)
