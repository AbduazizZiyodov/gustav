import logging
import os

DEBUG: bool = os.getenv("DEBUG", default="").lower() in ["y", "true", "yes"]

logging.basicConfig(
    level=logging.DEBUG if DEBUG else logging.INFO,
    format="[%(levelname)s] %(filename)s:%(lineno)d::%(funcName)s() +++ %(message)s",
)

LOG = logging.getLogger(__name__)
