# External model access and spending

No paid external-model call was made by this implementation. Task API expenditure is USD 0; this is not an assertion about the account's current balance or unrelated usage.

The user's authorization is limited to the USD 69 credit expiring on 13 November 2026. The separate USD 100 allocation must be preserved. A provider/model label, an API key, a subscription tier or a token budget does not establish which credit pool a request will consume.

Before any future paid run, verify the actual provider/endpoint/model, current unexpired credit balance and ordering, authoritative pricing, and a provider-enforced hard cap scoped to this task that cannot spill into the protected allocation. Account-wide concurrent spending and in-flight requests must be accounted for. Local token estimates alone do not satisfy this requirement. If this cannot be established, leave paid calls disabled.

A credential appeared in the chat, but was neither used nor persisted in this repository. Rotate that credential in the provider console; provision future credentials through the provider's secure local configuration or a secret manager, not chat, command arguments, source files or logs. There is intentionally no automatically executing API client in this prototype.
