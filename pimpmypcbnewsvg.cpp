#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>

#ifdef __CMAKE__
#include <tinyxml2.h>
#else
#include "tinyxml2/tinyxml2.h"
#endif

using namespace tinyxml2;
using std::cout;

std::unordered_map<std::string, std::string> _g_layers; // layer_name, color
std::unordered_map<std::string, std::string> _g_gw; // name, layer_name

struct layer
{
    std::string color;
    std::string _name;
    bool extractwhite;
};

struct output
{
    std::string fn_suffix;
    std::vector<layer> layers;
};
std::vector<output> g_outputs;

XMLNode *copy_nodes_selective(XMLNode *srcParent, XMLDocument *destDoc, const layer *lay);

int main(int argc, const char **argv)
{
    tinyxml2::XMLDocument cfg;
    XMLError err = cfg.LoadFile("pimpmypcbnewsvg.xml");
    if (err != XML_SUCCESS)
    {
        cout << "* Error: failed to load config: " << cfg.ErrorIDToName(err) << "\n";
        return 0;
    }
    {
        XMLElement *root = cfg.RootElement();
        // parse the "layers" section, it should contain a list of
        // all possible layers and their default colors
        XMLElement *ele0 = root->FirstChildElement("layers");
        XMLElement *ele1 = ele0->FirstChildElement();
        while (ele1 != nullptr)
        {
            //cout << "found: " << ele1->Name() << "\n";
            if (std::string(ele1->Name()).compare("layer") == 0)
            {
                std::string lyrname;
                std::string defclr;
                std::string gw;
                if (ele1->Attribute("name") != nullptr) { lyrname = ele1->Attribute("name"); }
                if (ele1->Attribute("defclr") != nullptr) { defclr = ele1->Attribute("defclr"); }
                if (ele1->Attribute("gw") != nullptr) { gw = ele1->Attribute("gw"); }
                if ((lyrname.empty()) || (defclr.empty()))
                {
                    cout << "* Error: cfg error at line " << ele1->GetLineNum() << "\n";
                    return 0;
                }
                _g_layers[lyrname] = defclr;
                if (gw.length() >= 1) { _g_gw[gw] = lyrname; }
            }
            ele1 = ele1->NextSiblingElement();
        }
        cout << "got " << _g_layers.size() << " layers and " << _g_gw.size() << " gw\n";
        //cout << "F.SilkS color is: " << _g_layers["F.SilkS"] << "\n";

        // parse a number of "output_file" sections:
        ele0 = ele0->NextSiblingElement("output_file");
        while (ele0 != nullptr)
        {
            output tmp;
            if (ele0->Attribute("suffix") != nullptr)
            {
                tmp.fn_suffix = ele0->Attribute("suffix");
                ele1 = ele0->FirstChildElement("layer");
                while (ele1 != nullptr)
                {
                    layer lyr;
                    if (ele1->Attribute("name") != nullptr)
                    {
                        // find if this is a valid layer
                        lyr._name = ele1->Attribute("name");
                        if (_g_layers.find(lyr._name) != _g_layers.end())
                        {
                            // okay, it's a normal layer
                            lyr.extractwhite = false;
                        }
                        else if (_g_gw.find(lyr._name) != _g_gw.end())
                        {
                            // okay, it's drill shapes that have to be extracted from a copper layer
                            lyr.extractwhite = true;
                        }
                        else
                        {
                            cout << "* Error: cfg: in output_file " << tmp.fn_suffix << ", unexpected layer: " << lyr._name << "\n";
                            return 0;
                        }
                    }
                    if (ele1->Attribute("color") != nullptr) { lyr.color = ele1->Attribute("color"); }
                    if ((lyr.color.length() == 0) || (lyr.color.compare("default") == 0))
                    {
                        if (lyr.extractwhite == false) { lyr.color = _g_layers[lyr._name]; }
                        else { lyr.color = "#333333"; }
                    }
                    cout << "    " << lyr.color << " " << (int)(lyr.extractwhite) << " " << lyr._name << "\n";
                    tmp.layers.push_back(lyr);
                    ele1 = ele1->NextSiblingElement("layer");
                }
                g_outputs.push_back(tmp);
            }
            else
            {
                cout << "* Error: cfg: output_file must contain a \"suffix\" attribute.\n";
                return 0;
            }
            cout << "  " << tmp.layers.size() << " layers pushed into " << tmp.fn_suffix << "\n";
            ele0 = ele0->NextSiblingElement("output_file");
        }
        //return 0;
    }
    /*
        two variants for calling convention
        "/path/fullname-F.Cu.svg" (drop a file onto the app, it could be any of the known layers, B.SilkS..)
        "-f /path/basename"

        TODO: add possibility to specify an alternative config file!
    */
    std::string fnb;

    if (argc < 2)
    {
        cout << "* Not enough arguments.\n";
        return 0;
    }
    if (argc == 2)
    {
        // a file was probably dropped onto the app
        // remove the .svg extension, then find if the last bits of the name contain any one of the
        // known layer names, and then remova that too, to get the "basename"
        //cout << argv[1] << "\n";
        fnb = argv[1];
        if (fnb.length() > 4)
        {
            std::string s = fnb.substr((fnb.length() - 4), 4);
            if (s.compare(".svg") == 0)
            {
                s = fnb.substr(0, (fnb.length() - s.length()));
                //size_t i = 0;
                bool found = false;
                for (auto it = _g_layers.begin(); it != _g_layers.end(); ++it)
                {
                    const std::string sw = it->first;
                    size_t n = sw.length();
                    std::string s2 = s.substr((s.length() - n), n);
                    if (s2.compare(sw) == 0)
                    {
                        // found one
                        found = true;
                        fnb = s.substr(0, (s.length() - n));
                        //cout << "found " << s2 << "\n" << fnb << "\n";
                        break;
                    }
                }
                if (!found)
                {
                    // maybe it's actually a base name, fall thru...
                }
                //cout << s << "\n";
            }
        }
    }
    else if (argc == 3)
    {
        std::string swi = argv[1];
        if (swi.compare("-f") == 0)
        {
            fnb = argv[2];
        }
        else
        {
            cout << "* Unknown command line switch: " << swi << "\n";
        }
    }
    else
    {
        cout << "* Bad command line arguments.\n";
    }
    if (fnb.length() <= 1) { return 0; }
    cout << "* Base filename: \"" << fnb << "\"\n";

    size_t docidx = 0;
    size_t layerN = 1;
    bool once_per_outDoc = true;
    bool abort = false;
    size_t outidx = 0;
    while (outidx < g_outputs.size())
    {
        output &rOut = g_outputs[outidx];
        std::string fnout = fnb + rOut.fn_suffix; //"merged.svg";
        cout << "* Preparing " << rOut.fn_suffix << " ...\n";
        tinyxml2::XMLDocument outDoc;
        docidx = 0;
        layerN = 1;
        once_per_outDoc = true;
        while (docidx < rOut.layers.size()) //NUM_LAYERS)
        {
            layer &rLay = rOut.layers.at(docidx);
            std::string fn;
            if (rLay.extractwhite)
            {
                cout << "  * Loading drill layer: " << rLay._name << " (" << _g_gw[rLay._name] << ")\n";
                fn = fnb + _g_gw[rLay._name] + ".svg";
            }
            else
            {
                cout << "  * Loading layer: " << rLay._name << "\n";
                fn = fnb + rLay._name + ".svg";
            }


            //cout << fn << '\n';
            tinyxml2::XMLDocument doc;
            tinyxml2::XMLError res = doc.LoadFile(fn.c_str());
            if (res == XML_SUCCESS)
            {
                tinyxml2::XMLNode *rootNode = doc.FirstChild();
                if (once_per_outDoc)
                {
                    once_per_outDoc = false;
                    // copy the nodes before the "svg" node into the outDoc
                    tinyxml2::XMLNode *tmpA = rootNode;
                    tinyxml2::XMLNode *tmpB = 0; //outDoc.FirstChild();
                    while (true)
                    {
                        if (tmpA == 0) { break; }
                        std::string val = tmpA->Value();
                        cout << "  * adding node: " << val << "\n";
                        tinyxml2::XMLNode *tmpele = tmpA->ShallowClone(&outDoc);
                        if (tmpB == 0)  { outDoc.InsertFirstChild(tmpele); }
                        else            { outDoc.InsertEndChild(tmpele); }

                        if (val.compare("svg") == 0) { break; }
                        tmpB = tmpA;
                        tmpA = tmpA->NextSibling();
                    }
                }
                // copy the elements under svg, append to the "svg" element of outDoc.. maybe into a named group
                tinyxml2::XMLNode *svgNode = doc.FirstChildElement("svg");
                tinyxml2::XMLNode *dstNode = outDoc.FirstChildElement("svg");
                tinyxml2::XMLNode *srcNode = svgNode->FirstChild();

                //srcNode = svgNode; // HACK HACK

                tinyxml2::XMLElement *ng = outDoc.NewElement("g");
                char tmp[32];
                sprintf(tmp, "layer%lu", layerN); ++layerN;
                ng->SetAttribute("id", tmp);
                ng->SetAttribute("inkscape:label", rLay._name.c_str()); //g_layers[docidx].c_str());
                ng->SetAttribute("inkscape:groupmode", "layer");
                dstNode->InsertEndChild(ng);
                dstNode = ng;
                size_t dbgs = 0, dbga = 0;
                while (true)
                {
                    if (srcNode == nullptr) { break; } // hm...
                    tinyxml2::XMLNode *tmpnode = copy_nodes_selective(srcNode, &outDoc, &rLay);
                    if (tmpnode != nullptr) { dstNode->InsertEndChild(tmpnode); ++dbga; } //docidx));
                    else { ++dbgs; }
                    srcNode = srcNode->NextSibling();

                }
                if (dbgs > 0)
                {
                    cout << "  copied: " << dbga << ", skipped: " << dbgs << "\n";
                }
            }
            else if (res == XML_ERROR_FILE_NOT_FOUND)
            {
                //cout << "    * N/A\n";
                // okay, move along
            }
            else
            {
                // oops, some other error... halt
                cout << "* XML error: " << doc.ErrorIDToName(res) << "\n* Aborting...\n";
                abort = true;
            }
            if (abort) { break; }
            ++docidx;
        }
        if (abort) { break; }
        else
        {
            // save the merged file
            tinyxml2::XMLError res = outDoc.SaveFile(fnout.c_str(), false);
            if (res != XML_SUCCESS)
            {
                abort = true;
                cout << "* Output ERROR: " << outDoc.ErrorIDToName(res) << "\n* Aborting...\n";
            }
        }
        ++outidx;
    }
    return 0;
}


// #############################################################################



XMLNode *copy_nodes_selective(XMLNode *srcParent, XMLDocument *destDoc, const layer *lay)
{
    // can't use this to copy sibling nodes of the parent
    //size_t copied = 0;
    //XMLNode *dstParent = srcParent->ShallowClone(destDoc); // return this at the end
    // ShallowClone() here means we don't check and thus can't skip the parent node itself.. not good
    // the parent node should be evaluated and copied (or not) like the others
    XMLNode *dstParent = nullptr;
    //cout << "** copy_nodes_selective: srcParent = " << srcParent->Value() << ", at line " << srcParent->GetLineNum() << "\n";

    // -------
    int64_t src_depth = 0;
    int64_t dst_depth = 0;
    bool work = true;
    tinyxml2::XMLNode *snode, *dnode, *snext, *dnext;
    snode = srcParent;
    dnode = dstParent;
    bool down = false;
    bool skip = false;
    while (work)
    {
        uint8_t clone = 0; // 1 = down, 2 = right
        if ((dstParent == nullptr) && (snode == srcParent))
        {
            // copy snode, use clone=1, don't insert it
            clone = 1;
            snext = snode;
        }
        else
        {
            snext = snode->FirstChild();
            down = false;
            if (skip == false)
            {
                if (snext != nullptr)
                {
                    // GOING RIGHT
                    ++src_depth;
                    // COPY IT, then loop around and try to fc() again
                    snode = snext; // hm
                    clone = 2;
                }
                else
                {
                    // this right here really means dead-end, can't go to the right anymore
                    // gotta try to go down (nextsibling())
                    // or left (parent())
                    down = true;
                }
            }
            else
            {
                down = true;
            }
        }
        // -----------

        while (down)
        {
            if (down)
            {
                // DOWN
                // can go down only once to find an existing element
                // can go multiple times left to f
                // TODO: check if we're trying to go down while snode==srcParent
                if (snode == srcParent)
                {
                    //cout << "\n<-- ** THE END 2 **\n";
                    work = false;
                    //clone = 1;
                    break;
                }
                snext = snode->NextSibling();
                if (snext != nullptr)
                {
                    //--dst_depth;
                    if (0)//(skip == false)
                    {
                        if (dnode == dstParent) { cout << "\n*wooops!*\n"; }
                        dnext = dnode->Parent();
                        --dst_depth;
                        dnode = dnext;
                    }
                    clone = 1;
                    break;
                }
            }
            {
                // LEFT
                // we reach here because we couldn't go DOWN anymore
                // if this node is the parent node - we shouldn't try to go left anymore too
                if (snode == srcParent)
                {
                    //cout << "<- ** THE END 1 **\n";
                    work = false;
                    break;
                }
                snext = snode->Parent();
                --src_depth;
                snode = snext;

                if (snode == srcParent)
                {
                    //cout << "<- ** THE END **\n";
                    work = false;
                    break;
                }
                {
                    if (dnode == dstParent) { cout << "\n*wooops!*\n"; }
                    dnext = dnode->Parent();
                    --dst_depth;
                    dnode = dnext;
                }
            }
        }
        if (work == false) { break; }
        snode = snext;
        skip = false;

        if (clone)
        {
            // possibly skip?
            {
                // process the element (snode) and also decide whether we want to keep it or skip it
                // idea: maybe skip empty group nodes?
                // ---------------------------------------------

                // i think "skip" should actually go down and left till another node is found.. so..
                tinyxml2::XMLElement *ele = snode->ToElement();
                bool keep = true;
                if (ele != nullptr)
                {
                    const char *p = ele->Attribute("style");
                    if (p != nullptr)
                    {
                        //cout << p << "\n";
                        std::string style = p;
                        //cout << "*** \"" << style << "\"\n";
                        std::vector<std::string> keywords = { {"fill:"}, {"stroke:"} };

                        size_t ikw = 0;
                        while (ikw < keywords.size())
                        {
                            auto pos0 = style.find(keywords[ikw]);
                            if (pos0 != std::string::npos)
                            {
                                pos0 += keywords[ikw].length(); // add the length

                                // find the ";" .. but there may not be any
                                auto pos1 = style.find(";", pos0);
                                if (pos1 != std::string::npos)
                                {
                                    auto len = (pos1-pos0);
                                    if (len > 14)
                                    {
                                        cout << "* warning: 0001\n";
                                    }
                                    //cout << "* f \"" << style.substr(pos0, len) << "\" >>> " << g_clrmap[layeridx] << "\n";
                                    {
                                        // check the original color
                                        std::string oc = style.substr(pos0, len);
                                        if (oc.compare("#FFFFFF") == 0)
                                        {
                                            if (lay->extractwhite == true)
                                            {
                                                // we need this
                                                keep = true;
                                                //cout << "keep\n";
                                            }
                                            else
                                            {
                                                keep = false;
                                                //cout << "skip\n";
                                            }
                                        }
                                        else
                                        {
                                            // black probably
                                            if (lay->extractwhite)
                                            {
                                                keep = false;
                                            }
                                        }
                                    }
                                    style.replace(pos0, len, lay->color);
                                }
                                else
                                {
                                    cout << "* err: 0001\n";
                                }
                            }
                            ++ikw;
                        }
                        {
                            // anything else to change? no? okay..
                        }
                        //cout << style << "\n";
                        ele->SetAttribute("style", style.c_str());
                        // wait, there seems to be one group in the beginning which has style and color, but also
                        // a "transform" attribute, and without it - the scaling seems wrong/undefined
                        if (ele->Attribute("transform") != nullptr) { keep = true; }

                        if (keep == false)
                        {
                            //ele->DeleteAttribute("style");
                            clone = false;
                            skip = true;
                            //cout << "*   " << snode->Value() << " (line " << snode->GetLineNum() << ") --skipping--\n";
                        }
                    }
                }
            }
            // -------
            if (clone == 2)
            {
                ++dst_depth;
                //cout  << "-> COPY: " << snode->Value() << " (" << src_depth << ":" << dst_depth << ") (line: " << snode->GetLineNum() << ")\n";
                tinyxml2::XMLNode *tmp = snode->ShallowClone(destDoc);
                dnode = dnode->InsertEndChild(tmp);
                if (dnode == nullptr)
                {
                    cout << "\nfeck\n";
                }
            }
            else if (clone == 1)
            {
                //++dst_depth;
                tinyxml2::XMLNode *tmp = snode->ShallowClone(destDoc);
                if ((dstParent == nullptr) && (snode == srcParent))
                {
                    // this must be the parent node, don't insert it
                    dstParent = tmp;
                    dnode = dstParent;
                }
                else
                {
                    //cout << "\\/ COPY: " << snode->Value() << " (" << src_depth << ":" << dst_depth << ") (line: " << snode->GetLineNum() << ")\n";
                    dnode = dnode->Parent()->InsertEndChild(tmp);
                }
                if (dnode == nullptr)
                {
                    cout << "\nfeck\n";
                }
            }
        }
        if ((skip) && (dstParent == nullptr))
        {
            // skipping parent node?
            work = false;
        }
    }
    // -------
    return dstParent;
}
