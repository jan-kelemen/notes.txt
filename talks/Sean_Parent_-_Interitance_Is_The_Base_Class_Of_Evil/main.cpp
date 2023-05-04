#include <iostream>
#include <memory>
#include <vector>

template <typename T>
void draw(T const& x, std::ostream& stream, std::size_t indent)
{
    stream << std::string(indent, ' ') << x << '\n';
}

class object_t
{
public:
    template <typename T>
    object_t(T x) 
        : self_{ std::make_shared<model<T>>(std::move(x)) } 
    {}
    
    friend void draw(object_t const& x, std::ostream& stream, std::size_t indent)
    {
        x.self_->call_draw(stream, indent);
    }
    
private:
    struct concept_t
    {
        virtual ~concept_t() = default;

        virtual void call_draw(std::ostream& stream, std::size_t indent) const = 0;
    };

    template <typename T>
    struct model : public concept_t
    {
        model(T x) : data_(std::move(x)) {} // Don't use braced initializer here! https://cplusplus.github.io/CWG/issues/2137.html

        void call_draw(std::ostream& stream, std::size_t indent) const override
        {
            draw(data_, stream, indent);
        }

        T data_;
    };
    
    std::shared_ptr<const concept_t> self_;
};


using document_t = std::vector<object_t>;

void draw(document_t const& doc, std::ostream& stream, std::size_t indent)
{
    stream << std::string(indent, ' ') << "<document>\n";

    for (object_t const& e : doc)
    {
        draw(e, stream, indent + 2);
    }

    stream << std::string(indent, ' ') << "</document>\n";
}

int main() {

    document_t doc;
    doc.emplace_back(1);
    doc.emplace_back(2);
    doc.emplace_back(doc);
    doc.emplace_back(5);
    doc.emplace_back(doc);

    draw(doc, std::cout, 0);
    
    return 0;
}
