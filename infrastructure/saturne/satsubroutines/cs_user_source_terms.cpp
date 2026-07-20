/*============================================================================
 * Additional user-defined source terms for variable equations.
 *============================================================================*/

/* code_saturne version 9.0 */

/*
  This file is part of code_saturne, a general-purpose CFD tool.

  Copyright (C) 1998-2025 EDF S.A.

  This program is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
  Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

/*----------------------------------------------------------------------------*/

#include "base/cs_defs.h"

/*----------------------------------------------------------------------------
 * Standard C library headers
 *----------------------------------------------------------------------------*/

#include <assert.h>
#include <math.h>

/*----------------------------------------------------------------------------
 * PLE library headers
 *----------------------------------------------------------------------------*/

#include <ple_coupling.h>

/*----------------------------------------------------------------------------
 * Local headers
 *----------------------------------------------------------------------------*/

#include "cs_headers.h"

/*----------------------------------------------------------------------------*/

BEGIN_C_DECLS

/*----------------------------------------------------------------------------*/
/*!
 * \file cs_user_source_terms.cpp
 *
 * \brief Additional source terms for variable equations.
 *
 * See \ref user_source_terms for examples.
 */
/*----------------------------------------------------------------------------*/

/*============================================================================
 * User function definitions
 *============================================================================*/

/*----------------------------------------------------------------------------*/
/*!
 * \brief Additional user-defined source terms for variable equations
 *        (momentum, scalars, turbulence...).
 *
 *  This function is called at each time step, for each relevant field.
 *  It is therefore necessary to
 *  test the value of the field id or name to separate
 *  the treatments of the different variables.
 *
 *  The additional source term is decomposed into an explicit part (st_exp) and
 *  an implicit part (st_imp) that must be provided here.
 *  The resulting equation solved by the code for a scalar f is:
 *
 *    \f[ \rho*volume*\frac{df}{dt} + .... = st\_imp*f + st\_exp \f]
 *
 *  Note that st_exp and st_imp are defined after the Finite Volume integration
 *  over the cells, so they include the "volume" term. More precisely:
 *    - st_exp is expressed in kg.[var]/s, where [var] is the unit of the
 *      variable.
 *      Its dimension is the one of the variable (3 for vectors)
 *    - st_imp is expressed in kg/s.
 *      Its dimension is 1 for scalars, 3x3 for vectors.
 *
 *  The st_exp and st_imp arrays are already initialized to 0 (or a value
 *  defined through the GUI or defined by a model) before entering
 *  the function. It is generally not useful to reset them here.
 *
 *  For stability reasons, code_saturne will not add -st_imp directly to the
 *  diagonal of the matrix, but Max(-st_imp,0). This way, the st_imp term is
 *  treated implicitely only if it strengthens the diagonal of the matrix.
 *  However, when using the second-order in time scheme, this limitation cannot
 *  be done anymore and -st_imp is added directly. The user should therefore
 *  check for the negativity of st_imp.
 *
 *  When using the second-order in time scheme, one should supply:
 *    - st_exp at time n
 *    - st_imp at time n+1/2
 *
 *  \warning
 *  \parblock
 *
 *   If the variable is a temperature, the resulting equation solved is:
 *
 *   rho*Cp*volume*dT/dt + .... = st_imp*T + st_exp
 *
 *  \endparblock
 *
 *  Note that st_exp and st_imp are defined after the Finite Volume integration
 *  over the cells, so they include the "volume" term. More precisely:
 *    - st_exp is expressed in W
 *    - st_imp is expressed in W/K
 *
 *  \par Steep source terms
 *  \parblock
 *
 *  In case of a complex, non-linear source term, say F(f), for variable f, the
 *  easiest method is to implement the source term explicitly.
 *
 *    df/dt = .... + F(f(n))
 *    where f(n) is the value of f at time tn, the beginning of the time step.
 *
 *  This yields:
 *    st_exp = volume*F(f(n))
 *    st_imp = 0
 *
 *  However, if the source term is potentially steep, this fully explicit
 *  method will probably generate instabilities. It is therefore wiser to
 *  partially implicit the term by writing:
 *
 *    df/dt = .... + dF/df*f(n+1) - dF/df*f(n) + F(f(n))
 *
 *  This yields:
 *    st_exp = volume*( F(f(n)) - dF/df*f(n) )
 *    st_imp = volume*dF/df
 *
 *  \endparblock
 *
 * \param[in, out]  domain   pointer to a cs_domain_t structure
 * \param[in]       f_id     field id of the variable
 * \param[out]      st_exp   explicit source term
 * \param[out]      st_imp   implicit part of the source term
 */
/*----------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>

/*---------------------------------------------------------------------------------*/
/* ustsns equivalent function (adds drag to trees)                                 */
/*---------------------------------------------------------------------------------*/
static void tree_drag(const cs_domain_t  *domain, int f_id,
            cs_real_t *st_exp, cs_real_t *st_imp)
{




  //Local Varaibles setup 
  const cs_field_t *f = cs_field_by_id(f_id);
  // current field (Velocity)
  const cs_real_t *i_face_surf = cs_glob_mesh_quantities->i_face_surf;
  // density 
  const cs_real_t *cpro_rom = CS_F_(rho)->val;
  const cs_real_t *vel = f->val;
  // setup local variables for the equation
  const cs_real_t ctraine = 0.9;
  const cs_real_t lad = 1.0;
  // get the tree cells 
  cs_lnum_t n_sel = 0;
  cs_lnum_t *sel_cells = (cs_lnum_t *) malloc(cs_glob_mesh->n_cells * sizeof(cs_lnum_t));
  bft_printf("DEBUG: calling selector $$familles_volume_arbre\n");
  cs_selector_get_cell_list("$$familles_volume_arbre", &n_sel, sel_cells); 
  
  //debug line
  bft_printf("\nTREE ZONE DEBUG: n_sel = %ld\n", n_sel);
  for (cs_lnum_t i = 0; i < CS_MIN(10, n_sel); i++) {
  bft_printf("cell[%ld] = %ld\n", i, sel_cells[i]);
  }
  /*------------------------------------------------------*/
  /* The equation                           */
  /*------------------------------------------------------*/
  // loops over the the cells changing the equation
  for (cs_lnum_t i = 0; i < n_sel; i++) {

    cs_lnum_t c = sel_cells[i];

    /* velocity components */
    cs_real_t u = vel[3*c + 0];
    cs_real_t v = vel[3*c + 1];
    cs_real_t w = vel[3*c + 2];

    /* velocity magnitude */
    cs_real_t v2 = sqrt(u*u + v*v + w*w);

    /* density */
    cs_real_t rho = cpro_rom[c];

    /*--------------------------------------------------*/
    /* source term (implicit sink)                      */
    /*--------------------------------------------------*/
    st_imp[c] = -rho * ctraine * v2 * lad;
  }
  BFT_FREE(sel_cells); 
}


/*---------------------------------------------------------------------------------*/
/* ustssc helper functions (add_physics())                                         */
/*---------------------------------------------------------------------------------*/
cs_real_t calculer_es(cs_lnum_t        i_cell, cs_real_t        w_ref, const cs_field_t *f_temp,
            const cs_field_t *f_pres, const cs_field_t *f_hum, const cs_field_t *f_rho){


  cs_real_t loc_p_a, loc_q_sat, loc_e_sat;
  cs_real_t loc_r_sto = 100.0;
  cs_real_t loc_r_aero = 100.0;

  cs_real_t T_calc = f_temp->val[i_cell];
  cs_real_t P_calc = 100000.0 + f_pres->val[i_cell];
  cs_real_t w_calc = f_hum->val[i_cell];
  static const cs_real_t LAD = 1.0;

  

  /* Saturation vapor pressure */
  if (T_calc > 100.0)
    loc_e_sat = 50000.0;
  else if (T_calc < -20.0)
    loc_e_sat = 100.0;
  else
    loc_e_sat = 610.7 * pow(1.0 + 1.414 * sin(T_calc * 3.14 / (3.0 * 180.0)), 8.827);

  loc_p_a = P_calc - loc_e_sat;

  loc_q_sat = 0.622 * loc_e_sat / (0.622 * loc_e_sat + loc_p_a);

  return LAD * (loc_q_sat - w_calc) / (loc_r_aero + loc_r_sto);
}

cs_real_t calculer_evap(cs_lnum_t i_cell, cs_real_t Flatent, const cs_field_t *f_temp)
{
  cs_real_t T_calc = f_temp->val[i_cell];
  cs_real_t evap_calc = Flatent / (2501000.0 - 2361.0 * T_calc);

  if (evap_calc <= 0.0)
    evap_calc = 0.0;

  return evap_calc;
}

cs_real_t calculer_surface(cs_lnum_t i_face)
{
  const cs_real_t *b_face_surf = cs_glob_mesh_quantities->b_face_surf;

  cs_real_t surf =
    sqrt(b_face_surf[3*i_face]     * b_face_surf[3*i_face] +
         b_face_surf[3*i_face + 1] * b_face_surf[3*i_face + 1] +
         b_face_surf[3*i_face + 2] * b_face_surf[3*i_face + 2]);

  return surf;
}


// ustssc equivalent , adds vegetation/trees heat + humidity exchange , applies the flux  to the walls and surfacs
void add_physics(cs_domain_t *domain, int f_id, cs_real_t *st_exp, cs_real_t *st_imp){
  CS_UNUSED(domain);
  CS_UNUSED(f_id);
  /* This function is divided into  parts, 
     1. initialization
     2. Calculation of heat source terms for trees
     3. Calculating moisture sources for trees
  */

  const cs_mesh_t *mesh = cs_glob_mesh;
  const cs_mesh_quantities_t *mq = cs_glob_mesh_quantities;
  // initalization
  cs_real_t LAD = 1.0;
  
  cs_real_t v, T_ext, w_ext;

  int rank = (cs_glob_rank_id < 0) ? 0 : cs_glob_rank_id;

  char file[1024];

  const char *chemin_data = "$$chemin_data";

  sprintf(file, "%s/v", chemin_data); //velocity
  FILE *fv = fopen(file, "r");
  if (fv == NULL) {
    bft_error(__FILE__, __LINE__, 0,
              "Rank %d cannot open file: %s",
              cs_glob_rank_id, file);
  }
  fscanf(fv, "%lf", &v);
  fclose(fv);

  sprintf(file, "%s/T", chemin_data); // temperature
  FILE *fT = fopen(file, "r");
  if (fT == NULL) {
    bft_error(__FILE__, __LINE__, 0,
              "Rank %d cannot open file: %s",
              cs_glob_rank_id, file);
  }
  fscanf(fT, "%lf", &T_ext);  
  fclose(fT);

  sprintf(file, "%s/w", chemin_data); // humidity
  FILE *fw = fopen(file, "r");
  if (fw == NULL) {
    bft_error(__FILE__, __LINE__, 0,
              "Rank %d cannot open file: %s",
              cs_glob_rank_id, file);
  }
  fscanf(fw, "%lf", &w_ext);
  fclose(fw);

  const cs_field_t *f_temp = cs_field_by_name("temperature");
  const cs_field_t *f_rho  = CS_F_(rho);
  const cs_field_t *f_hum = cs_field_by_name("user_2");
  const cs_field_t *f_pres = CS_F_(p);


  // source term on thermal scalar
  if (f_id == 1) {
    cs_lnum_t nlelt = 0;
    cs_lnum_t *lstelt = NULL;

    BFT_MALLOC(lstelt, mesh->n_cells, cs_lnum_t);

    cs_selector_get_cell_list("$$familles_volume_arbre", &nlelt, lstelt);

    for (cs_lnum_t i = 0; i < nlelt; i++) {

      cs_lnum_t c_id = lstelt[i];

      cs_real_t rho_air = f_rho->val[c_id];
      cs_real_t cp_air  = 1000.0;
      cs_real_t r_aero  = 100.0;

      cs_real_t temp = f_temp->val[c_id];

      cs_real_t Es = calculer_es(c_id, w_ext, f_temp, f_pres, f_hum, f_rho);

      cs_real_t H = rho_air * cp_air * (temp - T_ext) / r_aero;

      cs_real_t L = 2.501e6 - 2.361e3 * temp;

      cs_real_t Sh = -LAD * (H + L * Es);

      st_exp[c_id] = 0.0;
      st_exp[c_id] = cs_glob_mesh_quantities->cell_vol[c_id] * Sh;
    }  
  
    // Boundary faces
    FILE *fFs = NULL, *fFl = NULL;
    char fileFs[1024], fileFl[1024];

    snprintf(fileFs, sizeof(fileFs), "%s/Fsensible_%d", chemin_data, rank);
    fFs = fopen(fileFs, "r");

    if (fFs == NULL) {
      bft_error(__FILE__, __LINE__, 0, "Rank %d cannot open file: %s", cs_glob_rank_id, fileFs);
    }

    snprintf(fileFl, sizeof(fileFl), "%s/Flatent_%d", chemin_data, rank);
    fFl = fopen(fileFl, "r");

    if (fFl == NULL) {
       bft_error(__FILE__, __LINE__, 0, "Rank %d cannot open file: %s", cs_glob_rank_id, fileFl);
     }

    cs_lnum_t n_faces_b = 0;
    cs_lnum_t *lstfbr0 = NULL;

    BFT_MALLOC(lstfbr0, cs_glob_mesh->n_b_faces, cs_lnum_t);

    cs_selector_get_b_face_list("$$familles_murs", &n_faces_b, lstfbr0);

    const cs_real_t *b_surf = cs_glob_mesh_quantities->b_face_surf;
    const cs_lnum_t *b_face_cells = cs_glob_mesh->b_face_cells;

    for (cs_lnum_t i = 0; i < n_faces_b; i++) {

      cs_real_t Fsensible, Flatent;  
      fscanf(fFs, "%lf", &Fsensible);
      fscanf(fFl, "%lf", &Flatent);

      cs_lnum_t f_id = lstfbr0[i];
      cs_lnum_t c_id = b_face_cells[f_id];

      cs_real_t surf = calculer_surface(f_id);

      st_exp[c_id] = surf * Fsensible;

      if (Flatent > 0.0) {
        st_exp[c_id] -= Flatent * calculer_surface(f_id);
      }
    }

    /*======================================================*/
    /* 6. Internal faces (tree surface)
    /*======================================================*/

    cs_lnum_t n_faces_i = 0;
    cs_lnum_t *lstfac = NULL;

    BFT_MALLOC(lstfac, cs_glob_mesh->n_i_faces, cs_lnum_t);

    /* ⚠️ Placeholder: getfac equivalent */
    cs_selector_get_i_face_list("$$familles_surface_arbre", &n_faces_i, lstfac);

    const cs_real_t *i_surf = cs_glob_mesh_quantities->i_face_surf;
    const cs_lnum_t (*i_face_cells)[2] = cs_glob_mesh->i_face_cells;

    for (cs_lnum_t i = 0; i < n_faces_i; i++) {

      cs_real_t Fsensible;
      Fsensible = 0.0;  /* placeholder test value */
      //fscanf(fFs, "%lf", &Fsensible);

      cs_lnum_t f_id = lstfac[i];

      cs_real_t surf = i_surf[f_id];

      Fsensible *= surf;

      cs_lnum_t c1 = i_face_cells[f_id][0];
      cs_lnum_t c2 = i_face_cells[f_id][1];

      if (st_exp[c1] != 0.0)
        st_exp[c1] += Fsensible;

      if (st_exp[c2] != 0.0)
        st_exp[c2] += Fsensible * 0.5;

      st_imp[c1] = 0.0;
      st_imp[c2] = 0.0;
    }

    fclose(fFs);
    fclose(fFl);

    BFT_FREE(lstfbr0);
    BFT_FREE(lstfac);
    BFT_FREE(lstelt); 
  }

  

  if (f_id == 2) {

    //Cell selection (trees volume)
    cs_lnum_t nlelt = 0;
    cs_lnum_t *lstelt = NULL;

    BFT_MALLOC(lstelt, cs_glob_mesh->n_cells, cs_lnum_t);

    cs_selector_get_cell_list("$$familles_volume_arbre", &nlelt, lstelt);

    //Volume source term
    const cs_field_t *f_temp = CS_F_(t);

    for (cs_lnum_t i = 0; i < nlelt; i++) {

      cs_lnum_t c_id = lstelt[i];
      cs_real_t Es = calculer_es(c_id, w_ext, f_temp, CS_F_(p),cs_field_by_name("user_2"), CS_F_(rho));

      st_imp[c_id] = 0.0;
      st_exp[c_id] = cs_glob_mesh_quantities->cell_vol[c_id] * Es;
    }

    // File name construction (Flatent)
    FILE *fFl2 = NULL;
    char fileFl[1024];
 
    snprintf(fileFl, sizeof(fileFl), "%s/Flatent_%d", chemin_data, rank);
    fFl2 = fopen(fileFl, "r");
    if (fFl2 == NULL) {
      bft_error(__FILE__, __LINE__, 0, "Cannot open Flatent file: %s (rank %d)", fileFl, cs_glob_rank_id);
    }

    //Boundary faces (walls)
    cs_lnum_t nlfbr1 = 0;
    cs_lnum_t *lstfbr1 = NULL;

    BFT_MALLOC(lstfbr1, cs_glob_mesh->n_b_faces, cs_lnum_t);

    cs_selector_get_b_face_list("$$familles_murs", &nlfbr1, lstfbr1);

    const cs_lnum_t *b_face_cells = cs_glob_mesh->b_face_cells;

    const cs_real_t *b_surf = cs_glob_mesh_quantities->b_face_surf;

    for (cs_lnum_t i = 0; i < nlfbr1; i++) {

      cs_real_t Flatent;
      fscanf(fFl2, "%lf", &Flatent);

      if (Flatent > 0.0) {

        cs_lnum_t f_id = lstfbr1[i];
        cs_lnum_t c_id = b_face_cells[f_id];
        cs_real_t surf = b_surf[f_id];

        st_exp[c_id] = calculer_evap(c_id, Flatent, CS_F_(t)) * surf;
      }
    }

    fclose(fFl2);

    BFT_FREE(lstfbr1);
    BFT_FREE(lstelt);
  }

}

// ustske equivalent
void update_turbulence(cs_domain_t *domain, int f_id, cs_real_t *st_exp, cs_real_t *st_imp){
  CS_UNUSED(domain);
  CS_UNUSED(f_id);

  const cs_real_t ctraine = 0.9;
  const cs_real_t lad      = 1.0;
  const cs_real_t betap    = 1.0;
  const cs_real_t betad    = 5.03;
  const cs_real_t cquatre  = 0.78;
  const cs_real_t v_ref = 10.0;

  const cs_lnum_t n_cells = cs_glob_mesh->n_cells;

  const cs_real_3_t *vel = (const cs_real_3_t *)CS_F_(vel)->val;   /* velocity field */
  const cs_real_t *rho = CS_F_(rho)->val;                         /* density field */
  const cs_real_t *k = CS_F_(k)->val;                           /* turbulence k */

  cs_lnum_t nlelt = 0;
  cs_lnum_t *lstelt = NULL;

  BFT_MALLOC(lstelt, n_cells, cs_lnum_t);

  cs_selector_get_cell_list("$$familles_volume_arbre",&nlelt,lstelt);

  for (cs_lnum_t i = 0; i < nlelt; i++) {

    cs_lnum_t c = lstelt[i];
    cs_real_t u = vel[c][0];
    cs_real_t v = vel[c][1];
    cs_real_t w = vel[c][2];

    /* velocity magnitude */
    cs_real_t vmag = sqrt(u*u + v*v + w*w);
    cs_real_t v_norm = vmag / v_ref;
    /* smooth saturation to prevent blow-up */
    v_norm = v_norm / (1.0 + v_norm);
    /* bounded cubic term */
    cs_real_t v_cubic = v_norm * v_norm * v_norm;
    /* explicit source (bounded) */
    cs_real_t crk_exp = rho[c] * ctraine * betap * v_cubic;
    cs_real_t crk_imp = -rho[c] * betad * v_norm;

    st_exp[c] += crk_exp;
    st_imp[c] += crk_imp;

    if (k[c] > 1e-12) {
      cs_real_t cre_imp = (cquatre / k[c]) * (crk_exp + crk_imp * k[c]);
      st_imp[c] += cre_imp;
    }
  }

  BFT_FREE(lstelt);
}

// Main Function to run the source terms
void
cs_user_source_terms(cs_domain_t *domain, int f_id, cs_real_t *st_exp, cs_real_t *st_imp)
{
  bft_printf("\n*** SOURCE TERMS CALLED ***\n");
  CS_UNUSED(domain);
  CS_UNUSED(f_id);
  /* Calls the tree drag function uses the momentum sink (drag) equation */
  tree_drag(domain, f_id, st_exp, st_imp);
  add_physics(domain, f_id, st_exp, st_imp);
  update_turbulence(domain, f_id, st_exp, st_imp);

}
 
/*----------------------------------------------------------------------------*/

END_C_DECLS
