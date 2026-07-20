from dataclasses import dataclass
from functools import lru_cache
from pathlib import Path


@dataclass(frozen=True)
class SaturneTemplates:
    cs_user_source_terms: str
    cs_user_boundary_conditions: str
    cs_user_postprocessing: str
    saturne_xml: str


class SaturneTemplateRepository:
    def __init__(self, root: Path | None = None) -> None:
        self.root = root or Path(__file__).resolve().parent / "satsubroutines"

    @lru_cache(maxsize=1)
    def load(self) -> SaturneTemplates:
        return SaturneTemplates(
            cs_user_source_terms=(self.root / "cs_user_source_terms.cpp").read_text(encoding="utf-8"),
            cs_user_boundary_conditions=(self.root / "cs_user_boundary_conditions.f90").read_text(encoding="utf-8"),
            cs_user_postprocessing=(self.root / "cs_user_postprocess.cpp").read_text(encoding="utf-8"),
            saturne_xml=(self.root / "saturne3.xml").read_text(encoding="utf-8"),
        )