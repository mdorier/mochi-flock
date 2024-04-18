/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include "flock/flock-bootstrap.h"
#ifdef ENABLE_MPI
#include "flock/flock-bootstrap-mpi.h"
#endif
#include "view-serialize.h"

flock_return_t flock_group_view_init_from_self(
    margo_instance_id mid,
    uint16_t provider_id,
    flock_group_view_t* view)
{
    char      self_addr_str[256];
    hg_size_t self_addr_size = 256;
    hg_addr_t self_addr      = HG_ADDR_NULL;
    hg_return_t hret         = HG_SUCCESS;
    flock_return_t ret       = FLOCK_SUCCESS;

    hret = margo_addr_self(mid, &self_addr);
    if(hret != HG_SUCCESS) {
        ret = FLOCK_ERR_FROM_MERCURY;
        goto finish;
    }
    hret = margo_addr_to_string(mid, self_addr_str, &self_addr_size, self_addr);
    if(hret != HG_SUCCESS) {
        ret = FLOCK_ERR_FROM_MERCURY;
        goto finish;
    }

    flock_group_view_clear(view);

    if(!flock_group_view_add_member(view, 0, provider_id, self_addr_str)) {
        ret = FLOCK_ERR_ALLOCATION;
        goto finish;
    }

finish:
    margo_addr_free(mid, self_addr);
    return ret;
}

flock_return_t flock_group_view_init_from_file(
        margo_instance_id mid,
        const char* filename,
        flock_group_view_t* view)
{
    uint64_t credentials;
    return group_view_from_file(mid, filename, view, &credentials);
}

#ifdef ENABLE_MPI
flock_return_t flock_group_view_init_from_mpi(
        margo_instance_id mid,
        uint16_t provider_id,
        MPI_Comm comm,
        flock_group_view_t* view)
{
    char      self_addr_str[256];
    hg_size_t self_addr_size = 256;
    hg_addr_t self_addr      = HG_ADDR_NULL;
    hg_return_t hret         = HG_SUCCESS;
    flock_return_t ret       = FLOCK_SUCCESS;
    uint16_t* provider_ids   = NULL;
    char* addresses_buf      = NULL;
    int size, rank, mret;

    hret = margo_addr_self(mid, &self_addr);
    if(hret != HG_SUCCESS) {
        ret = FLOCK_ERR_FROM_MERCURY;
        goto finish;
    }
    hret = margo_addr_to_string(mid, self_addr_str, &self_addr_size, self_addr);
    if(hret != HG_SUCCESS) {
        ret = FLOCK_ERR_FROM_MERCURY;
        goto finish;
    }

    mret = MPI_Comm_rank(comm, &rank);
    if(mret != 0) {
        ret = FLOCK_ERR_FROM_MPI;
        goto finish;
    }
    mret = MPI_Comm_size(comm, &size);
    if(mret != 0) {
        ret = FLOCK_ERR_FROM_MPI;
        goto finish;
    }

    provider_ids = (uint16_t*)malloc(size*sizeof(provider_id));
    if(!provider_ids) {
        ret = FLOCK_ERR_ALLOCATION;
        goto finish;
    }
    addresses_buf = (char*)malloc(size*256);
    if(!addresses_buf) {
        ret = FLOCK_ERR_ALLOCATION;
        goto finish;
    }

    mret = MPI_Allgather(&provider_id, 1, MPI_UINT16_T, provider_ids, 1, MPI_UINT16_T, comm);
    if(mret != 0) {
        ret = FLOCK_ERR_FROM_MPI;
        goto finish;
    }

    mret = MPI_Allgather(self_addr_str, 256, MPI_CHAR, addresses_buf, 256, MPI_CHAR, comm);
    if(mret != 0) {
        ret = FLOCK_ERR_FROM_MPI;
        goto finish;
    }

    flock_group_view_clear(view);

    for(int i = 0; i < size; ++i) {
        if(!flock_group_view_add_member(view, (uint64_t)i, provider_ids[i], addresses_buf + 256*i)) {
            ret = FLOCK_ERR_ALLOCATION;
            goto finish;
        }
    }

finish:
    free(provider_ids);
    free(addresses_buf);
    margo_addr_free(mid, self_addr);
    return ret;
}
#endif