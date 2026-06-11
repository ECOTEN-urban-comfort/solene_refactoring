# Concrete XML-backed implementation of the configuration-loading contract.
#
# This module is the infrastructure adapter responsible for building a
# SimulationBootstrap from a simulation case folder.
#
# It performs the technical startup-loading tasks that used to be embedded
# directly in the legacy script, namely:
# - scanning the folder for required input files,
# - validating the presence of required XML files,
# - parsing sim_settings.xml,
# - converting raw XML values into typed settings,
# - deriving canonical runtime paths.
#
# In the refactored architecture, this module isolates XML parsing and file
# discovery from the application layer.

from pathlib import Path
from xml.dom import minidom

from application.ports.configuration_provider import ConfigurationProvider
from config.external_tools import build_tools_from_bin_dir
from config.runtime import build_runtime_paths
from domain.simulation_definition import SimulationBootstrap, InputFiles, SimulationSettings


class XmlConfigurationProvider(ConfigurationProvider):
    """
    This is the concrete first implementation of the `ConfigurationProvider` port.

    What is transferred here from the original code:
    ------------------------------------------------
    1. From `Simulation.py`
       - scanning the simulation folder to find the input files
       - reading `sim_settings.xml`
       - extracting startup variables from XML

    2. From `xmlFile.py`
       - the *idea* of XML tag-based reading
       - not the whole original class structure

    The original code loads configuration directly inside the simulation startup
    script. That makes the loading mechanism inseparable from the rest of the run.

    XML parsing and filesystem scanning are technical details of a specific input
    source. They should not live in the application service layer.
    """

    def load(self, sim_folder: Path) -> SimulationBootstrap:
        """
        Load one simulation case from the given folder.
        """
        sim_folder = Path(sim_folder).resolve()

        med_file = self._find_single_file(sim_folder, ".med")
        meteo_file = self._find_single_file(sim_folder, ".csv")

        sim_settings_file = self._require_file(sim_folder / "sim_settings.xml")
        famille_file = self._require_file(sim_folder / "famille.xml")
        materiau_file = self._require_file(sim_folder / "materiau.xml")

        # Load typed settings from sim_settings.xml.
        settings = self._load_settings(sim_settings_file)

        # Derive canonical runtime paths.
        paths = build_runtime_paths(sim_folder)

        external_tools = build_tools_from_bin_dir(
            Path("/home/sol_user/solene_refactoring/legacy/native/common_c/bin")
        )

        # New bootstrap shape: all discovered source files are grouped under
        # InputFiles instead of being spread as loose top-level fields.
        input_files = InputFiles(
            med_file=med_file,
            meteo_file=meteo_file,
            sim_settings_file=sim_settings_file,
            famille_file=famille_file,
            materiau_file=materiau_file,
        )

        return SimulationBootstrap(
            settings=settings,
            input_files=input_files,
            paths=paths,
            external_tools=external_tools,
        )

    def _find_single_file(self, folder: Path, suffix: str) -> Path:
        """
        Find exactly one file with the given suffix in the folder.

        This preserves the legacy startup assumption that there is exactly:
            - one MED geometry file
            - one meteo CSV file
        """
        matches = sorted(
            path for path in folder.iterdir()
            if path.is_file() and path.suffix.lower() == suffix.lower()
        )

        if not matches:
            raise FileNotFoundError(f"No {suffix} file found in {folder}")

        if len(matches) > 1:
            raise ValueError(
                f"Expected exactly one {suffix} file in {folder}, found {len(matches)}"
            )

        return matches[0]

    def _require_file(self, file_path: Path) -> Path:
        """
        Ensure a specific required input file exists.

        For files such as sim_settings.xml, famille.xml, and materiau.xml,
        the role is defined by the exact filename, not only by suffix.
        """
        if not file_path.is_file():
            raise FileNotFoundError(f"Missing required file: {file_path}")
        return file_path

    def _load_settings(self, xml_path: Path) -> SimulationSettings:
        """
        Load SimulationSettings from sim_settings.xml.
        """
        if not xml_path.is_file():
            raise FileNotFoundError(f"Missing settings file: {xml_path}")

        doc = minidom.parse(str(xml_path))

        def text(tag: str) -> str:
            nodes = doc.getElementsByTagName(tag)
            if not nodes or nodes[0].firstChild is None:
                raise ValueError(f"Missing XML tag: {tag}")
            return nodes[0].firstChild.data.strip()

        return SimulationSettings(
            begin_day=int(text("begin_day")),
            begin_month=int(text("begin_month")),
            begin_hour=int(text("begin_hour")),
            end_day=int(text("end_day")),
            end_month=int(text("end_month")),
            end_hour=int(text("end_hour")),
            latitude=float(text("latitude")),
            longitude=float(text("longitude")),
            surface_model=text("surface_model"),
            air_model=text("air_model"),
            ts_coupl=int(text("ts_coupl")),
            iter_init=int(text("iter_init")),
            iter_foll=int(text("iter_foll")),
            cores_used=int(text("cores_used")),
        )