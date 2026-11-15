import logging
import os

__all__ = "DEBUG", "LOG"

DEBUG: bool = os.getenv("DEBUG", default="").lower() in ["y", "true", "yes"]

logging.basicConfig(
    level=logging.DEBUG if DEBUG else logging.INFO,
    format="[%(levelname)s | %(funcName)s():%(lineno)d] %(message)s",
)

LOG = logging.getLogger(__name__)
