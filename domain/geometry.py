from dataclasses import dataclass
from pathlib import Path

@dataclass(frozen=True)
class PreparedGeometryInputs:
    staged_med_file: Path
    staged_famille_file: Path
    staged_materiau_file: Path

    sauvegarde_dir: Path
    simul_sol_dir: Path

    @property
    def geom_med_cpl(self) -> Path:
        return self.sauvegarde_dir / "geom_med.cpl"

    @property
    def geom_sol_cpl(self) -> Path:
        return self.sauvegarde_dir / "geom_sol.cpl"

    @property
    def scene_cir(self) -> Path:
        return self.simul_sol_dir / "scene_triangle.cir"

    @property
    def masque_cir(self) -> Path:
        return self.simul_sol_dir / "scene_masque.cir"

@dataclass(frozen=True)
class SoleneGeometryArtifacts:
    geom_med_cpl: Path
    geom_sol_cpl: Path
    scene_cir: Path
    masque_cir: Path
    n_sol_triangles: int