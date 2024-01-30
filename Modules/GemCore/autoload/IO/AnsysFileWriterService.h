#pragma once

#include <mitkAbstractFileWriter.h>

class AnsysFileWriterService : public mitk::AbstractFileWriter
{
public:
    AnsysFileWriterService(void);
    virtual ~AnsysFileWriterService(void);

    using mitk::AbstractFileWriter::Write;
    virtual void Write(void) override;

private:
    AnsysFileWriterService(const AnsysFileWriterService &other);
    virtual AnsysFileWriterService* Clone() const override;

    us::ServiceRegistration<mitk::IFileWriter> m_ServiceReg;
};