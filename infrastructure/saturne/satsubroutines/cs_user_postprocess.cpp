/*============================================================================
 * Define postprocessing output.
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

#include "stdlib.h"
#include "string.h"

/*----------------------------------------------------------------------------
 * Local headers
 *----------------------------------------------------------------------------*/

#include "cs_headers.h"

/*----------------------------------------------------------------------------*/

BEGIN_C_DECLS

/*============================================================================
 * Local (user defined) function definitions
 *============================================================================*/

/*============================================================================
 * User function definitions
 *============================================================================*/

/*----------------------------------------------------------------------------*/
/*!
 * \brief Define post-processing writers.
 *
 * The default output format and frequency may be configured, and additional
 * post-processing writers allowing outputs in different formats or with
 * different format options and output frequency than the main writer may
 * be defined.
 */
/*----------------------------------------------------------------------------*/

void
cs_user_postprocess_writers(void)
{

}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Define post-processing meshes.
 *
 * The main post-processing meshes may be configured, and additional
 * post-processing meshes may be defined as a subset of the main mesh's
 * cells or faces (both interior and boundary).
 */
/*----------------------------------------------------------------------------*/

void
cs_user_postprocess_meshes(void)
{
  // this function selects what faces to output in the mesh, instead of outputting whatever 
  const int n_writers = 1;

  /* Associate with default writer */
  const int writer_ids[] = {-1};

  cs_post_define_surface_mesh(
      100,                       /* mesh id */
      "solene_surfaces",         /* mesh name */
      "$$familles_surface_arbre",                      /* interior criteria */
      "$$familles_murs",         /* boundary criteria */
      false,                     /* add_groups */
      false,                     /* auto_variables */
      n_writers,
      writer_ids);

}

/*----------------------------------------------------------------------------*/
/*!
 * \brief Define monitoring probes and profiles.
 *
 * Profiles are defined as sets of probes.
 */
/*----------------------------------------------------------------------------*/

void
cs_user_postprocess_probes(void)
{

}

/*----------------------------------------------------------------------------*/
/*!
 * \brief User function for output of values on a post-processing mesh.
 *
 * \param[in]       mesh_name    name of the output mesh for the current call
 * \param[in]       mesh_id      id of the output mesh for the current call
 * \param[in]       cat_id       category id of the output mesh for the
 *                               current call
 * \param[in]       probes       pointer to associated probe set structure if
 *                               the mesh is a probe set, null otherwise
 * \param[in]       n_cells      local number of cells of post_mesh
 * \param[in]       n_i_faces    local number of interior faces of post_mesh
 * \param[in]       n_b_faces    local number of boundary faces of post_mesh
 * \param[in]       n_vertices   local number of vertices faces of post_mesh
 * \param[in]       cell_list    list of cells (0 to n-1) of post-processing
 *                               mesh
 * \param[in]       i_face_list  list of interior faces (0 to n-1) of
 *                               post-processing mesh
 * \param[in]       b_face_list  list of boundary faces (0 to n-1) of
 *                               post-processing mesh
 * \param[in]       vertex_list  list of vertices (0 to n-1) of
 *                               post-processing mesh
 * \param[in]       ts           time step status structure, or nullptr
 */
/*----------------------------------------------------------------------------*/

#include "cs_headers.h"
#include <math.h>
#include <stdio.h>


/*----------------------------------------------------------------------------*/

void
cs_user_postprocess_values(const char            *mesh_name,
                           int                    mesh_id,
                           int                    cat_id,
                           cs_probe_set_t        *pset,
                           cs_lnum_t              n_cells,
                           cs_lnum_t              n_i_faces,
                           cs_lnum_t              n_b_faces,
                           cs_lnum_t              n_vertices,
                           const cs_lnum_t       *cell_list,
                           const cs_lnum_t       *i_face_list,
                           const cs_lnum_t       *b_face_list,
                           const cs_lnum_t       *vtx_list,
                           const cs_time_step_t  *ts)
{

  if (mesh_id != 100)
      return;

  // to check if the post_processor_mesh function is working (the one above)
  // printf("\n");
  // printf("mesh_name = %s\n", mesh_name);
  // printf("mesh_id   = %d\n", mesh_id);
  // printf("cat_id    = %d\n", cat_id);
  // printf("n_b_faces = %d\n", (int)n_b_faces);
  // printf("n_i_faces = %d\n", (int)n_i_faces);
  // printf("\n");

  // to test the fields in code saturne because humidity is a user scalar
  // the humididty scalar is called user_2 in code saturne 9 
  // for (int i = 0; i < cs_field_n_fields(); i++) {
  //     printf("%s\n", cs_field_by_id(i)->name);
  // }

  const cs_mesh_t *m = cs_glob_mesh;
  const cs_mesh_quantities_t *mq = cs_glob_mesh_quantities;

  /* Fields */
  const cs_field_t *vel = cs_field_by_name("velocity");
  const cs_field_t *temp_f = cs_field_by_name("temperature");
  const cs_field_t *hs_f = cs_field_by_name("user_2"); // humidity

  const cs_real_t *vel_val  = vel->val;
  const cs_real_t *temp_val = temp_f->val;
  const cs_real_t *hs_val   = hs_f->val;

  /* Connectivity */
  const cs_lnum_t *b_face_cells = m->b_face_cells;
  const cs_lnum_2_t *i_face_cells = m->i_face_cells;

  const cs_real_3_t *b_face_cog = mq->b_face_cog;
  const cs_real_3_t *i_face_cog = mq->i_face_cog;

  // echange files handling (the output of code saturne)
  // rank is the how many cores are being used (MPI according to gpt)
  int rank = (cs_glob_rank_id < 0) ? 0 : cs_glob_rank_id;

  char f_temp_name[256];
  char f_vel_name[256];
  char f_hc_name[256];
  char f_hs_name[256];

  snprintf(f_temp_name, sizeof(f_temp_name), "%s/temperature_%d", "$$chemin_echange", rank);
  snprintf(f_vel_name, sizeof(f_vel_name), "%s/vitesse_%d", "$$chemin_echange", rank);
  snprintf(f_hc_name, sizeof(f_hc_name),"%s/h_conv_%d", "$$chemin_echange", rank);
  snprintf(f_hs_name, sizeof(f_hs_name), "%s/hs_%d", "$$chemin_echange", rank);

  FILE *f_temp = fopen(f_temp_name, "w");
  FILE *f_vel  = fopen(f_vel_name,  "w");
  FILE *f_hc   = fopen(f_hc_name,   "w");
  FILE *f_hs   = fopen(f_hs_name,   "w");

  /* BOUNDARY FACES            */
  for (cs_lnum_t i = 0; i < n_b_faces; i++) {

    cs_lnum_t f_id = b_face_list[i];
    cs_lnum_t c_id = b_face_cells[f_id];

    cs_real_t u = vel_val[3*c_id + 0];
    cs_real_t v = vel_val[3*c_id + 1];
    cs_real_t w = vel_val[3*c_id + 2];

    cs_real_t vmag = sqrt(u*u + v*v + w*w);
    cs_real_t Hconv = 11.8 + 4.2 * vmag;

    cs_real_t temp = temp_val[c_id];
    cs_real_t hs   = hs_val[c_id];

    fprintf(f_temp, "%f %f %f %f\n", b_face_cog[f_id][0], b_face_cog[f_id][1], b_face_cog[f_id][2], temp);
    fprintf(f_vel, "%f %f %f %f\n", b_face_cog[f_id][0], b_face_cog[f_id][1], b_face_cog[f_id][2], vmag);
    fprintf(f_hc, "%f %f %f %f\n", b_face_cog[f_id][0], b_face_cog[f_id][1], b_face_cog[f_id][2], Hconv);
    fprintf(f_hs, "%f %f %f %f\n", b_face_cog[f_id][0], b_face_cog[f_id][1], b_face_cog[f_id][2], hs);
  }

  /* INTERIOR FACES            */
  for (cs_lnum_t i = 0; i < n_i_faces; i++) {

    cs_lnum_t f_id = i_face_list[i];

    cs_lnum_t c1 = i_face_cells[f_id][0];
    cs_lnum_t c2 = i_face_cells[f_id][1];

    /* cell 1 */
    cs_real_t u1 = vel_val[3*c1 + 0];
    cs_real_t v1 = vel_val[3*c1 + 1];
    cs_real_t w1 = vel_val[3*c1 + 2];

    /* cell 2 */
    cs_real_t u2 = vel_val[3*c2 + 0];
    cs_real_t v2 = vel_val[3*c2 + 1];
    cs_real_t w2 = vel_val[3*c2 + 2];

    cs_real_t vmag1 = sqrt(u1*u1 + v1*v1 + w1*w1);
    cs_real_t vmag2 = sqrt(u2*u2 + v2*v2 + w2*w2);

    cs_real_t vmag = 0.5 * (vmag1 + vmag2);
    cs_real_t Hconv = 11.8 + 4.2 * vmag;

    cs_real_t temp = 0.5 * (temp_val[c1] + temp_val[c2]);
    cs_real_t hs   = 0.5 * (hs_val[c1]   + hs_val[c2]);

    fprintf(f_temp, "%f %f %f %f\n",i_face_cog[f_id][0],i_face_cog[f_id][1],i_face_cog[f_id][2],temp);
    fprintf(f_vel, "%f %f %f %f\n",i_face_cog[f_id][0],i_face_cog[f_id][1],i_face_cog[f_id][2],vmag);
    fprintf(f_hc, "%f %f %f %f\n",i_face_cog[f_id][0],i_face_cog[f_id][1],i_face_cog[f_id][2],Hconv);
    fprintf(f_hs, "%f %f %f %f\n",i_face_cog[f_id][0],i_face_cog[f_id][1],i_face_cog[f_id][2],hs);
  }

  fclose(f_temp);
  fclose(f_vel);
  fclose(f_hc);
  fclose(f_hs);

}

/*----------------------------------------------------------------------------*/
/*!
 * Override default frequency or calculation end based output.
 *
 * This allows fine-grained control of activation or deactivation,
 *
 * \param[in]  nt_max_abs  maximum time step number
 * \param[in]  nt_cur_abs  current time step number
 * \param[in]  t_cur_abs   absolute time at the current time step
 */
/*----------------------------------------------------------------------------*/

void
cs_user_postprocess_activate(int     nt_max_abs,
                             int     nt_cur_abs,
                             double  t_cur_abs)
{
  CS_UNUSED(nt_max_abs);
  CS_UNUSED(nt_cur_abs);
  CS_UNUSED(t_cur_abs);
}

/*----------------------------------------------------------------------------*/

END_C_DECLS
