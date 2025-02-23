import os
import logging


DEBUG: bool = os.getenv("DEBUG", "").lower() in ["y", "true", "yes"]

logging.basicConfig(
    level=logging.DEBUG if DEBUG else logging.INFO,
    format="%(asctime)s %(levelname)s: %(message)s",
    datefmt="%Y-%m-%d/%H:%M",
)


log = logging.getLogger(__name__)
