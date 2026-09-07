from infrastructure.coupling.dat_val_bridge import DatValBridge
from infrastructure.solene.hdfFile import CplFile
from infrastructure.solene.sol_file import read_val, write_val
import time

class LegacyCouplingGateway:
    """
    Technical bridge for the preserved legacy Solene <-> Code_Saturne
    coupling operations.
    """

    def initialize_exchange(
        self,
        *,
        sol_command,
        sat_command,
        sol_env,
        geometry,
        step_index: int,
        previous_ts: str,
        current_ts: str,
        processors: int,
        base_paths: dict,
    ) -> None:
        """
        First Saturne -> Solene exchange after the initial Saturne run.

        Legacy equivalent:
        - connection_dat2val_multipro_bis(...)
        - dat2val Vair
        - dat2val Tair
        - dat2val hs
        - calculate hc = 3.8 * ws + 5.7
        - define Solene meteo
        - run first coupled Ts/EnergieBat step
        """

        # Legacy runtime geometry is reconstructed locally from disk.
        cpl_file = CplFile(str(geometry.geom_sol_cpl))
        cpl_file.charger_geom()

        geom_sol = cpl_file.geom

        if geom_sol is None:
            raise ValueError(
                f"Could not load geom_sol from {geometry.geom_sol_cpl}"
            )

        self.bridge = DatValBridge(geom_sol)
        bridge = self.bridge

        # Legacy:
        # connection_dat2val_multipro_bis(
        #     SatCommand.chemins['echange'] + '/h_conv',
        #     n_proc_saturne,
        # )
        bridge.connect_multipro(
            sat_command.chemins["echange"] + "/h_conv",
            processors,
        )

        # ---------------------------------------------------------
        # Saturne -> Solene
        # ---------------------------------------------------------

        # Vair -> wind speed
        bridge.dat2val(
            sat_command.nom_dat["Vair"],
            base_paths["v"] + "_" + current_ts,
            n_proc=processors,
        )

        # Tair -> air temperature
        bridge.dat2val(
            sat_command.nom_dat["Tair"],
            base_paths["Tair"] + "_" + current_ts,
            n_proc=processors,
        )

        # hs -> relative humidity input
        bridge.dat2val(
            sat_command.nom_dat["hs"],
            base_paths["HR"] + "_" + current_ts,
            n_proc=processors,
        )

        # ---------------------------------------------------------
        # Convective heat-transfer coefficient
        #
        # Legacy:
        # ws = read_val(...)
        # hc = 3.8 * ws + 5.7
        # write_val(...)
        # ---------------------------------------------------------

        ws = read_val(
            base_paths["v"] + "_" + current_ts,
            geom_sol,
        )

        hc = 3.8 * ws + 5.7

        write_val(
            base_paths["hc"] + "_" + current_ts,
            geom_sol,
            hc,
        )

        # ---------------------------------------------------------
        # First coupled Solene step
        # ---------------------------------------------------------

        # Keep this exactly as in legacy:
        # only hc is redirected here.
        sol_command.var["hc"] = (
            base_paths["hc"] + "_" + current_ts
        )

        sol_env.definir_meteo(
            step_index,
            veg=True,
        )

        sol_command.simulation_Ts_EnergieBat_new(
            previous_ts,
            current_ts,
            simulation_batiment=False,
            simulation_vegetation=True,
            meteo=False,
            terminal=False,
        )

    def run_step(
        self,
        *,
        sol_command,
        sat_command,
        sol_env,
        meteo,
        step_index: int,
        previous_ts: str,
        current_ts: str,
        iterations: int,
        processors: int,
        base_paths: dict,
    ) -> None:
        """
        Execute one legacy coupled Solene <-> Code_Saturne timestep.
        """

        bridge = getattr(self, "bridge", None)

        if bridge is None:
            raise RuntimeError(
                "Coupling bridge is not initialized. "
                "initialize_exchange() must run before run_step()."
            )

        geom_sol = bridge.geom_sol

        print(f"\033[91m STARTED THE LOOP {step_index} \033[0m")

        # ---------------------------------------------------------
        # Restore original Solene paths.
        # ---------------------------------------------------------

        sol_command.var["hc"] = base_paths["hc"]
        sol_command.var["Tair"] = base_paths["Tair"]
        sol_command.var["HR"] = base_paths["HR"]
        sol_command.carac["v"] = base_paths["v"]

        # Legacy:
        # sim.solEnv.definir_meteo(i, veg=True)

        sol_env.definir_meteo(
            step_index,
            veg=True,
        )

        print("TimeStep Number:", current_ts)

        # =========================================================
        # SOLENE -> SATURNE
        # =========================================================

        # Legacy:
        # val2dat_multipro(
        #     flux_latent + '_' + avant + '.val',
        #     SatCommand.chemins['data'] + '/Flatent',
        #     n_proc_saturne,
        # )

        bridge.val2dat(
            sol_command.var["flux_latent"]
            + "_"
            + previous_ts
            + ".val",
            sat_command.chemins["data"] + "/Flatent",
            n_proc=processors,
        )

        # ---------------------------------------------------------
        # Calculate sensible convective flux.
        #
        # Legacy retained `hc` from the previous iteration.
        # Here the same value is read from the previous timestep
        # VAL file which initialize_exchange()/previous run_step()
        # already wrote.
        # ---------------------------------------------------------

        tair = read_val(
            base_paths["Tair"]
            + "_"
            + previous_ts
            + ".val",
            geom_sol,
        )

        tse = read_val(
            sol_command.var["Tse"]
            + "_"
            + previous_ts
            + ".val",
            geom_sol,
        )

        hc = read_val(
            base_paths["hc"]
            + "_"
            + previous_ts
            + ".val",
            geom_sol,
        )

        flux_convectif = hc * (tse - tair)

        flux_convectif_file = (
            sol_command.var["flux_convectif"]
            + "_"
            + previous_ts
            + ".val"
        )

        write_val(
            flux_convectif_file,
            geom_sol,
            flux_convectif,
        )

        bridge.val2dat(
            flux_convectif_file,
            sat_command.chemins["data"] + "/Fsensible",
            n_proc=processors,
        )

        # =========================================================
        # CODE_SATURNE
        # =========================================================

        sat_command.definir_meteo(meteo)

        sat_command.definir_restart()

        sat_command.ajouter_iterations(
            iterations
        )

        print(
            "\033[91m "
            f"GOING TO CALL LANCER SIM FOR ITER NO: {step_index} "
            "\033[0m"
        )

        sat_command.launch_simulation(
            terminal=False,
        )

        # Keep legacy behaviour for now.
        time.sleep(3)

        sat_command.follow_simulation()

        # =========================================================
        # SATURNE -> SOLENE
        # =========================================================

        # IMPORTANT:
        # No connect_multipro() here.
        #
        # Legacy also created the mapping only once before the loop.
        # self.bridge preserves that same mapping.

        bridge.dat2val(
            sat_command.nom_dat["Vair"],
            base_paths["v"] + "_" + current_ts,
            n_proc=processors,
        )

        bridge.dat2val(
            sat_command.nom_dat["Tair"],
            base_paths["Tair"] + "_" + current_ts,
            n_proc=processors,
        )

        bridge.dat2val(
            sat_command.nom_dat["hs"],
            base_paths["HR"] + "_" + current_ts,
            n_proc=processors,
        )

        # ---------------------------------------------------------
        # Calculate hc for the new timestep.
        # ---------------------------------------------------------

        ws = read_val(
            base_paths["v"]
            + "_"
            + current_ts,
            geom_sol,
        )

        hc = 3.8 * ws + 5.7

        write_val(
            base_paths["hc"]
            + "_"
            + current_ts,
            geom_sol,
            hc,
        )

        # ---------------------------------------------------------
        # Point Solene variables to current coupled timestep.
        # ---------------------------------------------------------

        sol_command.var["hc"] = (
            base_paths["hc"]
            + "_"
            + current_ts
        )

        sol_command.var["Tair"] = (
            base_paths["Tair"]
            + "_"
            + current_ts
        )

        sol_command.var["HR"] = (
            base_paths["HR"]
            + "_"
            + current_ts
        )

        sol_command.carac["v"] = (
            base_paths["v"]
            + "_"
            + current_ts
        )

        # =========================================================
        # SOLENE
        # =========================================================

        sol_command.simulation_Ts_EnergieBat_new(
            previous_ts,
            current_ts,
            simulation_batiment=False,
            simulation_vegetation=True,
            meteo=False,
            terminal=False,
        )

        print(
            "\t sim_ts : de %s a %s"
            % (
                previous_ts,
                current_ts,
            )
        )