/******************************************************************************
* Copyright (c) 2020, Hobu Inc. (info@hobu.co)
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following
* conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in
*       the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of Hobu, Inc. or Flaxen Geo Consulting nor the
*       names of its contributors may be used to endorse or promote
*       products derived from this software without specific prior
*       written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
* OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
* OF SUCH DAMAGE.
****************************************************************************/

#include <pdal/pdal_test_main.hpp>

#include <pdal/Filter.hpp>
#include <pdal/StageFactory.hpp>
#include <pdal/util/Bounds.hpp>

namespace pdal
{

namespace
{
    size_t g_count;
}

void exec1(const std::string& where, size_t expKeep, size_t expViews,
    Filter::WhereMergeMode mm = Filter::WhereMergeMode::Auto)
{
    StageFactory factory;

    Stage *r = factory.createStage("readers.faux");
    Options ro;
    ro.add("count", 100);
    ro.add("bounds", BOX3D(0, 0, 100, 99, 9.9, 199));
    ro.add("mode", "ramp");
    r->setOptions(ro);

    class TestFilter : public Filter
    {
        std::string getName() const
        { return "filters.test"; }

        void filter(PointView& v)
        {
            g_count = v.size();
        }
    };

    TestFilter f;
    Options fo;
    fo.add("where", where);
    fo.add("where_merge", mm);
    f.setOptions(fo);
    f.setInput(*r);

    PointTable t;
    f.prepare(t);
    PointViewSet s = f.execute(t);
    EXPECT_EQ(s.size(), expViews);
    size_t total = 0;
    for (auto vp : s)
        total += vp->size();
    EXPECT_EQ(total, 100);
    EXPECT_EQ(g_count, expKeep);
}

void exec2(const std::string& where, size_t expKeep, size_t expViews,
    Filter::WhereMergeMode mm = Filter::WhereMergeMode::Auto)
{
    StageFactory factory;

    Stage *r = factory.createStage("readers.faux");
    Options ro;
    ro.add("count", 100);
    ro.add("bounds", BOX3D(0, 0, 100, 99, 9.9, 199));
    ro.add("mode", "ramp");
    r->setOptions(ro);

    class TestFilter : public Filter
    {
        std::string getName() const
        { return "filters.test"; }

        void filter(PointView& v)
        {
            g_count = v.size();
            v.setField(Dimension::Id::X, v.size(), 1);
        }
    };

    TestFilter f;
    Options fo;
    fo.add("where", where);
    fo.add("where_merge", mm);
    f.setOptions(fo);
    f.setInput(*r);

    PointTable t;
    f.prepare(t);
    PointViewSet s = f.execute(t);
    EXPECT_EQ(s.size(), expViews);
    size_t total = 0;
    for (auto vp : s)
        total += vp->size();
    EXPECT_EQ(total, 101);
    EXPECT_EQ(g_count, expKeep);
}

void exec3(const std::string& where, size_t expKeep, size_t expViews,
    Filter::WhereMergeMode mm = Filter::WhereMergeMode::Auto)
{
    StageFactory factory;

    Stage *r = factory.createStage("readers.faux");
    Options ro;
    ro.add("count", 100);
    ro.add("bounds", BOX3D(0, 0, 100, 99, 9.9, 199));
    ro.add("mode", "ramp");
    r->setOptions(ro);

    class TestFilter : public Filter
    {
        std::string getName() const
        { return "filters.test"; }

        PointViewSet run(PointViewPtr v)
        {
            g_count = v->size();
            PointViewSet s;
            s.insert(v);
            s.insert(v->makeNew());
            return s;
        }
    };

    TestFilter f;
    Options fo;
    fo.add("where", where);
    fo.add("where_merge", mm);
    f.setOptions(fo);
    f.setInput(*r);

    PointTable t;
    f.prepare(t);
    PointViewSet s = f.execute(t);
    EXPECT_EQ(s.size(), expViews);
    size_t total = 0;
    for (auto vp : s)
        total += vp->size();
    EXPECT_EQ(total, 100);
    EXPECT_EQ(g_count, expKeep);
}

TEST(WhereTest, t1)
{
    exec1("X<50", 50, 1);
    exec1("X<50 && Y < 2.5", 25, 1);
    exec1("X<50 && Y < 2.5", 25, 1, Filter::WhereMergeMode::True);
    exec1("X<50 && Y < 2.5", 25, 2, Filter::WhereMergeMode::False);

    exec2("X<50", 50, 2);
    exec2("X<50 && Y < 2.5", 25, 2);
    exec2("X<50 && Y < 2.5", 25, 1, Filter::WhereMergeMode::True);
    exec2("X<50 && Y < 2.5", 25, 2, Filter::WhereMergeMode::False);

    exec3("X<50", 50, 3);
    exec3("X<50 && Y < 2.5", 25, 3);
    exec3("X<50 && Y < 2.5", 25, 2, Filter::WhereMergeMode::True);
    exec3("X<50 && Y < 2.5", 25, 3, Filter::WhereMergeMode::False);
}

} // namespace pdal
