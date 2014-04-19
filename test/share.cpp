/*
 *  This file is part of clearskies_core.

 *  clearskies_core is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  clearskies_core is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.

 *  You should have received a copy of the GNU Lesser General Public License
 *  along with clearskies_core.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "cs/share.hpp"
#include "cs/utils.hpp"
#include "utils.hpp"
#include <boost/test/unit_test.hpp>
#include "cs/boost_fs_fwd.hpp"
#include <utility>
#include <iostream>

using namespace std;
using namespace cs::share;
using cs::File;

#if 0

struct F
{
    F():
        old_cerr_rdbuf(cerr.rdbuf())
    {
        cerr.rdbuf(os.rdbuf());
    }

    ~F()
    {
        cerr.rdbuf(old_cerr_rdbuf);
    }
    streambuf* old_cerr_rdbuf;
    ostringstream os;

};

BOOST_FIXTURE_TEST_SUITE(suite, F)
#endif



BOOST_AUTO_TEST_CASE(Share_test_01)
{
    /*
     * Check basic invariants about the share itself.
     *
     */
    Tmpdir tmp;
    string share_id;
    string peer_id;
    string psk_rw;
    string psk_ro;
    string psk_untrusted;
    {
        Share share(tmp.tmpdir.string(), tmp.dbpath.string());
        create_tree(tmp.tmpdir);
        share_id = share.m_share_id;
        peer_id = share.m_peer_id;
        psk_rw = share.m_psk_rw;
        psk_ro = share.m_psk_ro;
        psk_untrusted = share.m_psk_untrusted;
        BOOST_CHECK(! share.m_share_id.empty());
        BOOST_CHECK(! share.m_peer_id.empty());
        BOOST_CHECK(! share.m_psk_rw.empty());
        BOOST_CHECK(! share.m_psk_ro.empty());
        BOOST_CHECK(! share.m_psk_untrusted.empty());
    }
    {
        // Check that the state of the share is the same when creating the class again. It should
        // read the settings stored on the share's database
        Share share(tmp.tmpdir.string(), tmp.dbpath.string());
        BOOST_CHECK_EQUAL(share_id, share.m_share_id);
        BOOST_CHECK_EQUAL(peer_id, share.m_peer_id);
        BOOST_CHECK_EQUAL(psk_rw, share.m_psk_rw);
        BOOST_CHECK_EQUAL(psk_ro, share.m_psk_ro);
        BOOST_CHECK_EQUAL(psk_untrusted, share.m_psk_untrusted);
    }
}

BOOST_AUTO_TEST_CASE(tail_test)
{
    BOOST_CHECK_EQUAL(get_tail(bfs::path("a/b/c"), 2).string(), "b/c");
    BOOST_CHECK_EQUAL(get_tail(bfs::path("a/b/c"), 1).string(), "c");
    BOOST_CHECK_EQUAL(get_tail(bfs::path("/a/b/c"), 4).string(), "/a/b/c");
    BOOST_CHECK_EQUAL(get_tail(bfs::path("/a/b/c/"), 1).string(), ".");
    BOOST_CHECK_EQUAL(get_tail(bfs::path("/a/b/c/d"), 1).string(), "d");
}

BOOST_AUTO_TEST_CASE(share_insert_mfile)
{
    REDIRECT_CERR;
    const int stderrcpy = dup(2);
    close(2);
    {
        MFile f;
        f.path = "omg/a/path";
        f.mtime = "12392";
        f.size = 69;
        f.mode = 01777;

        Tmpdir tmp;
        Share share(tmp.tmpdir.string(), tmp.dbpath.string());

        auto f_none = share.get_file_info("argsgs");
        BOOST_CHECK(! f_none);

        share.insert_mfile(f);
        BOOST_CHECK_THROW(share.insert_mfile(f), sqlite3pp::database_error);

        f_none = share.get_file_info("argsgs");
        BOOST_CHECK(! f_none);

        auto f_ = share.get_file_info(f.path);
        BOOST_CHECK(f_);
        BOOST_CHECK_EQUAL(f_->path, f.path);
        BOOST_CHECK_EQUAL(f_->mtime, f.mtime);
        BOOST_CHECK_EQUAL(f_->size, f.size);
        BOOST_CHECK_EQUAL(f_->mode, f.mode);
        BOOST_CHECK_EQUAL(f_->checksum, f.checksum);
        BOOST_CHECK_EQUAL(f_->deleted, f.deleted);
        cerr << "\nThe following error message is expected: statement::~statement: sqlite3_finalize returned with error..." << endl;
        cerr << endl;
    }
    dup2(stderrcpy, 2);
    REDIRECT_CERR_END;
}



BOOST_AUTO_TEST_CASE(share_state_0)
{
    /*
     * Check that the state of the share is what is expected after adding some files, deleting one,
     * and doing a re-scan
     */
    Tmpdir tmp;
    Share share(tmp.tmpdir.string(), tmp.dbpath.string());
    create_tree(tmp.tmpdir);

    for (const auto& file: share)
        BOOST_CHECK(file.checksum.empty());

    share.scan();
    while(share.scan_step()) {};

    {
        vector<MFile> files;
        size_t nfiles = 0;
        for (const auto& file: share)
        {
            //cout << file.path << endl;
            ++nfiles;
            //cout << file.checksum << endl;
            //cout << file.mtime << endl;
            files.emplace_back(file);
            BOOST_CHECK(! file.to_checksum);
            BOOST_CHECK(file.updated);
            BOOST_CHECK(! file.checksum.empty());
        }
        BOOST_CHECK_EQUAL(nfiles, 3);

        // REMOVE ONE FILE
        bfs::remove(bfs::path(tmp.tmpdir) / files.at(0).path);
    }


    // Re-scan
    share.scan();
    while(share.scan_step()) {};


    // Check manifest state after file removal
    {
        vector<MFile> files;
        size_t nfiles = 0;
        for (const auto& file: share)
        {
            ++nfiles;
            files.push_back(file);
        }
        BOOST_CHECK_EQUAL(nfiles, 3);
        for (size_t i = 0; i < 3; ++i)
        {
            const auto fpath = tmp.tmpdir / files.at(i).path;
            //cout << fpath.string() << endl;
            BOOST_CHECK(! files.at(i).to_checksum);
            if (i == 0)
            {
                BOOST_CHECK(! bfs::exists(fpath));
                BOOST_CHECK(! files.at(i).mtime.empty());
                BOOST_CHECK(files.at(i).deleted);
                BOOST_CHECK(files.at(i).size == 0u);
                BOOST_CHECK(files.at(i).mode == 0u);
                BOOST_CHECK(files.at(i).checksum.empty());
            }
            else
            {
                BOOST_CHECK(bfs::exists(fpath));
                BOOST_CHECK(! files.at(i).mtime.empty());
                BOOST_CHECK(! files.at(i).deleted);
                BOOST_CHECK(! files.at(i).checksum.empty());
            }

            //BOOST_CHECK(files.at(i).last_changed_rev);
            BOOST_CHECK(! files.at(i).last_changed_by.empty());
        }
    }
}


BOOST_AUTO_TEST_CASE(FrozenManifest_test_0)
{
    // We can't use std algorithms because the iterators are currently not copyable due to the
    // references @sa Share_iterator @sa FrozenManifestIterator
    // Maybe it would be better to use ptrs so they are copyable.
    Tmpdir tmp;
    Share share(tmp.tmpdir.string(), tmp.dbpath.string());
    create_tree(tmp.tmpdir);
    share.scan();
    while(share.scan_step()) {};



    {
        vector<MFile> manifest;
        for (const auto& file: share)
            manifest.emplace_back(file);

        vector<MFile> frozen_manifest;
        FrozenManifest fm = share.get_updates("peer_01");
        for (const auto& file: fm)
            frozen_manifest.emplace_back(file);
        BOOST_CHECK(manifest == frozen_manifest);

        // create another file, the frozen manifest should not change
        bfs::path f = tmp.tmpdir / "newfile";
        bfs::ofstream f_os(f);
        f_os << "omg I'm new" << endl;
        f_os.close();


        share.scan();
        while(share.scan_step()) {};

        vector<MFile> manifest_2;
        for (const auto& file: share)
            manifest_2.emplace_back(file);
        BOOST_CHECK(manifest != manifest_2);

        vector<MFile> frozen_manifest_2;
        for (const auto& file: fm)
            frozen_manifest_2.emplace_back(file);

        BOOST_CHECK(manifest == frozen_manifest);
        BOOST_CHECK(manifest == frozen_manifest_2);
        BOOST_CHECK(frozen_manifest == frozen_manifest_2);

        // it should throw if we try to freeze again for the same peer
        BOOST_CHECK_THROW(share.get_updates("peer_01"), std::runtime_error);

        // it should be able to freeze for another peer
        FrozenManifest fm_02 = share.get_updates("peer_02");
        vector<MFile> frozen_manifest_p02;
        for (const auto& file: fm_02)
            frozen_manifest_p02.emplace_back(file);

        // it should have the changes in manifest 2
        BOOST_CHECK(manifest_2 == frozen_manifest_p02);
    }
    // it should be able to freeze for the same peer, since we don't have it frozen anymore (out of
    // scope)
    vector<MFile> manifest;
    for (const auto& file: share)
        manifest.emplace_back(file);
    FrozenManifest fm = share.get_updates("peer_01");
    vector<MFile> frozen_manifest;
    for (const auto& file: fm)
        frozen_manifest.emplace_back(file);

    BOOST_CHECK(manifest == frozen_manifest);
}


BOOST_AUTO_TEST_CASE(Share_get_mfiles_by_content_test)
{
    Tmpdir tmp;
    Share share(tmp.tmpdir.string(), tmp.dbpath.string());
    create_file(tmp.tmpdir / "a", "a content");
    create_file(tmp.tmpdir / "aa", "a content");
    create_file(tmp.tmpdir / "b" / "a", "a content");

    share.scan();
    while(share.scan_step()) {};

    auto f = share.get_file_info("a");
    BOOST_CHECK(f);
    auto mfiles = share.get_mfiles_by_content2(f->checksum);
    BOOST_CHECK_EQUAL(mfiles.size(), 3u);
    BOOST_CHECK(mfiles[0].up_to_date);
}

#if 0
BOOST_AUTO_TEST_SUITE_END()
#endif

