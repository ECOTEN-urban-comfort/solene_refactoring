!-------------------------------------------------------------------------------

!                      code_saturne version 9.0
!                      ------------------------
! This file is part of code_saturne, a general-purpose CFD tool.
!
! Copyright (C) 1998-2024 EDF S.A.
!
! This program is free software; you can redistribute it and/or modify it under
! the terms of the GNU General Public License as published by the Free Software
! Foundation; either version 2 of the License, or (at your option) any later
! version.
!
! This program is distributed in the hope that it will be useful, but WITHOUT
! ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
! FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
! details.
!
! You should have received a copy of the GNU General Public License along with
! this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
! Street, Fifth Floor, Boston, MA 02110-1301, USA.

!-------------------------------------------------------------------------------

!===============================================================================
! Function:
! ---------

!> \file cs_user_boundary_conditions.f90
!>
!> \brief User subroutine which fills boundary conditions arrays
!> (\c icodcl, \c rcodcl) for solved variables.
!-------------------------------------------------------------------------------

!-------------------------------------------------------------------------------
! Arguments
!______________________________________________________________________________.
!  mode           name          role                                           !
!______________________________________________________________________________!
!> \param[in]     nvar          total number of variables
!> \param[in]     nscal         total number of scalars
!> \param[out]    icodcl        boundary condition code
!> \param[in]     itrifb        unused (null)
!> \param[in,out] itypfb        boundary face types
!> \param[in,out] izfppp        boundary face zone number
!> \param[in]     dt            time step (per cell)
!> \param[in,out] rcodcl        boundary condition values:
!>                               - rcodcl(1) value of the dirichlet
!>                               - rcodcl(2) value of the exterior exchange
!>                                 coefficient (infinite if no exchange)
!>                               - rcodcl(3) value flux density
!_______________________________________________________________________________

subroutine cs_f_user_boundary_conditions &
 ( nvar   , nscal  ,                                              &
   icodcl , itrifb , itypfb , izfppp ,                            &
   dt     ,                                                       &
   rcodcl )

!===============================================================================

!===============================================================================
! Module files
!===============================================================================

use paramx
use numvar
use optcal
use cstphy
use cstnum
use entsor
use parall
use period
use ppppar
use ppthch
use coincl
use ppincl
use ppcpfu
use atchem
use atincl
use atsoil
use ctincl
use mesh
use field
use iso_c_binding
use cs_c_bindings

!===============================================================================

implicit none

! Arguments

integer          nvar   , nscal

integer          icodcl(nfabor,nvar)
integer          itrifb(*), itypfb(nfabor)
integer          izfppp(nfabor)

double precision dt(ncelet)
double precision rcodcl(nfabor,nvar,3)

! Local variables

! INSERT_VARIABLE_DEFINITIONS_HERE

integer          ifac, iel, ii, ivar
integer          ilelt, nlelt

integer, allocatable, dimension(:) :: lstelt
integer, allocatable, dimension(:) :: lstelt2
integer, allocatable, dimension(:) :: lstelt3

double precision uref2, z_lim, norm_surfbo, u_norm, dir, t_meteo
double precision u_vec(3)
double precision prosca

double precision tfbr, hc, flux, tx_hmdt
integer          nlelt2, nlelt3


! wind profiles at the entrance
integer          i_profil, io
double precision zz(500), vv(500)

! =========================
! User parameters
! =========================

integer PROFIL
character(len=256) :: CHEMIN_DATA = '$$chemin_data/'
character(len=1024) :: FAMILLES_SAT_INLET = '$$familles_sat_inlet'
character(len=1024) :: FAMILLES_MURS = '$$familles_murs'
character(len=1024) :: FAMILLES_SAT_SYM = '$$familles_sat_sym'


PROFIL = 0

!write(*,*) '>>> Boundary Condition fortran is Called <<<'

!===============================================================================

! =========================
! Allocation
! =========================

allocate(lstelt(nfabor))
allocate(lstelt2(nfabor))
allocate(lstelt3(nfabor))

! =========================
! Read meteo data
! =========================

open(unit=11, file=trim(CHEMIN_DATA)//'v') ! Reference wind speed  m/s
read(11,*) uref2
close(11)

open(unit=12, file=trim(CHEMIN_DATA)//'direction') ! Wind direction  degrees
read(12,*) dir
close(12)

open(unit=13, file=trim(CHEMIN_DATA)//'T') ! Air temperature  °C
read(13,*) t_meteo
close(13)

open(unit=14, file=trim(CHEMIN_DATA)//'z0') ! Reference height / roughness scaling height  meters
read(14,*) z_lim
close(14)

open(unit=15, file=trim(CHEMIN_DATA)//'w') ! taux humidité / humidity content  kg/kg
read(15,*) tx_hmdt 
close(15)

! =========================
! Wind direction vector
! =========================

u_vec(1) = -sin(dir * acos(-1.d0) / 180.d0)
u_vec(2) = -cos(dir * acos(-1.d0) / 180.d0)
u_vec(3) = 0.d0

! =========================
! Optional wind profile
! =========================

if (PROFIL .eq. 1) then

  open(unit=55, file=trim(CHEMIN_DATA)//'profilV', status='old')

  i_profil = 1
  do
    read(55, *, iostat=io) zz(i_profil), vv(i_profil)
    if (io < 0) exit
    i_profil = i_profil + 1
  end do

  close(55)

endif

! =========================
! INLET BC
! =========================

call getfbr(FAMILLES_SAT_INLET, nlelt, lstelt)

do ilelt = 1, nlelt

  ifac = lstelt(ilelt)
  iel  = ifabor(ifac)

  prosca = surfbo(1,ifac)*u_vec(1) + surfbo(2,ifac)*u_vec(2)

  if (prosca < 0.d0) then

    itypfb(ifac) = ientre

    norm_surfbo = sqrt(surfbo(1,ifac)**2 + surfbo(2,ifac)**2 + surfbo(3,ifac)**2)

    if (PROFIL .eq. 1) then

      i_profil = 2
      do
        if (zz(i_profil) > cdgfbo(3,ifac)) then

          u_norm = ((vv(i_profil-1)-vv(i_profil))*cdgfbo(3,ifac) &
                   + zz(i_profil-1)*vv(i_profil) &
                   - zz(i_profil)*vv(i_profil-1)) &
                   / (zz(i_profil-1)-zz(i_profil))
          exit
        endif

        i_profil = i_profil + 1
      end do

    else

      u_norm = uref2 * (((cdgfbo(3,ifac)+30.d0)/z_lim)**0.3d0)

    endif

    ! Velocity
    rcodcl(ifac,iu,1) = u_vec(1)*u_norm
    rcodcl(ifac,iv,1) = u_vec(2)*u_norm
    rcodcl(ifac,iw,1) = 0.d0

    ! Turbulence
    rcodcl(ifac,iep,1) = 0.09d0*(0.0033d0*(uref2**2))**1.5d0 &
                         /(0.42d0*cdgfbo(3,ifac))

    rcodcl(ifac,ik,1)  = 0.0033d0*(uref2**2)

    ! Scalars
    do ivar = 1, nscal

      icodcl(ifac,isca(ivar)) = 1
      rcodcl(ifac,isca(ivar),1) = 0.d0
      rcodcl(ifac,isca(ivar),2) = 0.d0

    enddo

    rcodcl(ifac,isca(iscalt),1) = t_meteo
    rcodcl(ifac,isca(2),1)      = tx_hmdt

  elseif (prosca > 0.d0) then

    itypfb(ifac) = isolib

  else

    itypfb(ifac) = isymet

  endif

end do

! =========================
! WALL BC
! =========================

call getfbr(FAMILLES_MURS, nlelt2, lstelt2)

do ilelt = 1, nlelt2

  ifac = lstelt2(ilelt)

  itypfb(ifac) = iparoi

  icodcl(ifac,isca(1)) = 5
  rcodcl(ifac,isca(1),1) = 0.d0
  rcodcl(ifac,isca(1),2) = 0.d0

end do

! =========================
! SYMMETRY BC
! =========================

call getfbr(FAMILLES_SAT_SYM, nlelt3, lstelt3)

do ilelt = 1, nlelt3
  ifac = lstelt3(ilelt)
  itypfb(ifac) = isymet
end do
!--------
! Formats
!--------

!2010 format(F10.2,2X) ! unused in the new version

!----
! End
!----

deallocate(lstelt)  ! temporary array for boundary faces selection
deallocate(lstelt2)
deallocate(lstelt3)





return
end subroutine cs_f_user_boundary_conditions
